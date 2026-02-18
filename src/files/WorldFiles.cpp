#include "WorldFiles.h"

#include <cassert>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <string>

#include "files.h"
#include "rle.h"
#include "binary_io.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../window/Camera.h"
#include "../math/voxmaths.h"
#include "../logger/Logger.h"
#include "../content/Content.h"
#include "../voxels/Block.h"
#include "../world/World.h"
#include "../coders/json.h"
#include "../constants.h"

// Конвертирует 4 байта в целое число
inline int bytes2Int(const ubyte* src, size_t offset){
	return (src[offset] << 24) | (src[offset + 1] << 16) | (src[offset + 2] << 8) | (src[offset + 3]);
}

// Конвертирует целое число в 4 байта
inline void int2Bytes(int value, ubyte* dest, size_t offset){
	dest[offset] = (char) ((value >> 24) & 0xFF);
	dest[offset + 1] = (char) ((value >> 16) & 0xFF);
	dest[offset + 2] = (char) ((value >> 8) & 0xFF);
	dest[offset + 3] = (char) ((value >> 0) & 0xFF);
}

WorldRegion::WorldRegion() {
	chunksData = new ubyte*[RegionConsts::VOLUME]{};
	sizes = new uint32_t[RegionConsts::VOLUME]{};
}

WorldRegion::~WorldRegion() {
	for (uint i = 0; i < RegionConsts::VOLUME; ++i) {
		delete[] chunksData[i];
	}
	delete[] sizes;
	delete[] chunksData;
}

void WorldRegion::put(uint x, uint z, ubyte* data, uint32_t size) {
	size_t chunk_index = z * RegionConsts::SIZE + x;
	delete[] chunksData[chunk_index];
	chunksData[chunk_index] = data;
	sizes[chunk_index] = size;
}

ubyte* WorldRegion::get(uint x, uint z) {
	return chunksData[z * RegionConsts::SIZE + x];
}

uint WorldRegion::getSize(uint x, uint z) {
	return sizes[z * RegionConsts::SIZE + x];
}

// Конструктор
WorldFiles::WorldFiles(std::filesystem::path directory, bool generatorTestMode) : directory(directory), generatorTestMode(generatorTestMode){
    // Проверяем существование директории. Если её нет, то пытаемся создать
    if (!std::filesystem::is_directory(directory)) {
		std::filesystem::create_directory(directory);
	}

    // Инициализируем буферы
	compressionBuffer = new ubyte[CHUNK_DATA_LEN * 2];
}

// Деструктор 
WorldFiles::~WorldFiles(){
    // Освобождаем буферы
	delete[] compressionBuffer;

    // Осовбождаем регионы из памяти
	for (auto it : regions){
		delete it.second;
	}
	regions.clear();
}

WorldRegion* WorldFiles::getRegion(int x, int z) {
	auto found = regions.find(glm::ivec2(x, z));
	if (found == regions.end()) return nullptr;
	return found->second;
}

ubyte* WorldFiles::compress(ubyte* src, size_t srclen, size_t& len) {
	len = extrle::encode(src, srclen, compressionBuffer);
	ubyte* data = new ubyte[len];
	for (size_t i = 0; i < len; ++i) {
		data[i] = compressionBuffer[i];
	}
	return data;
}

ubyte* WorldFiles::decompress(ubyte* src, size_t srclen, size_t dstlen) {
	ubyte* decompressed = new ubyte[dstlen];
	extrle::decode(src, srclen, decompressed);
	return decompressed;
}

// Помещает данные чанка в кэш памяти
void WorldFiles::put(Chunk* chunk){
	assert(chunk != nullptr);

    // Вычисляем координаты региона
	int regionX = floordiv(chunk->chunk_x, RegionConsts::SIZE);
    int regionZ = floordiv(chunk->chunk_z, RegionConsts::SIZE);

    WorldRegion* region = getRegion(regionX, regionZ);
	if (region == nullptr) {
		region = new WorldRegion();
		regions[glm::ivec2(regionX, regionZ)] = region;
	}
	region->setUnsaved(true);

	int localX = chunk->chunk_x - (regionX * RegionConsts::SIZE);
	int localZ = chunk->chunk_z - (regionZ * RegionConsts::SIZE);

	std::unique_ptr<ubyte[]> chunk_data (chunk->encode());
	size_t compressedSize;
	ubyte* data = compress(chunk_data.get(), CHUNK_DATA_LEN, compressedSize);
	region->put(localX, localZ, data, compressedSize);
}

std::filesystem::path WorldFiles::getRegionsFolder() const {
	return directory/std::filesystem::path("regions");
}

// Генерирует имя файла для региона с заданными координатами
std::filesystem::path WorldFiles::getRegionFile(int x, int z) const {
	return directory/std::filesystem::path(std::to_string(x) + "_" + std::to_string(z) + ".bin");
}

// Генерирует имя файла, в котором записана информация об игроке
std::filesystem::path WorldFiles::getPlayerFile() const {
	return directory/std::filesystem::path("player.json");
}

std::filesystem::path WorldFiles::getWorldFile() const {
    return directory/std::filesystem::path("world.json");
}

std::filesystem::path WorldFiles::getIndicesFile() const {
	return directory/std::filesystem::path("indices.json");
}

// Получает данные чанка из кэша или файла
ubyte* WorldFiles::getChunk(int x, int z){
	// Вычисляем координаты региона
	int regionX = floordiv(x, RegionConsts::SIZE);
    int regionZ = floordiv(z, RegionConsts::SIZE);

    // Локальные координаты внутри региона
	int localX = x - (regionX * RegionConsts::SIZE);
	int localZ = z - (regionZ * RegionConsts::SIZE);

	WorldRegion* region = getRegion(regionX, regionZ);
	if (region == nullptr) {
		region = new WorldRegion();
		regions[glm::ivec2(regionX, regionZ)] = region;
	}

	ubyte* data = region->get(localX, localZ);
	if (data == nullptr) {
        uint32_t size;
		data = readChunkData(x, z, size, getRegionFile(regionX, regionZ));
        if (data) region->put(localX, localZ, data, size);
    }

    if (data) return decompress(data, region->getSize(localX, localZ), CHUNK_DATA_LEN);

    return nullptr;
}

// Читает чанк непосредственно из файла
ubyte* WorldFiles::readChunkData(int x, int z, uint32_t& length, std::filesystem::path filename){
    if (generatorTestMode) return nullptr;

    // Вычисляем координаты региона
	int regionX = floordiv(x, RegionConsts::SIZE);
    int regionZ = floordiv(z, RegionConsts::SIZE);

    // Локальные координаты внутри региона
	int localX = x - (regionX * RegionConsts::SIZE);
	int localZ = z - (regionZ * RegionConsts::SIZE);

	int chunk_index = localZ * RegionConsts::SIZE + localX;

    // Открываем файл
	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()) return nullptr;

	input.seekg(0, std::ios::end);
	size_t file_size = input.tellg();
    size_t table_offset = file_size - RegionConsts::VOLUME * 4;

	uint32_t offset;
	input.seekg(table_offset + chunk_index * 4);
	input.read((char*)(&offset), 4);
	offset = bytes2Int((const ubyte*)(&offset), 0);
	if (offset == 0){
		input.close();
		return nullptr;
	}
	input.seekg(offset);
	input.read((char*)(&offset), 4);
	length = bytes2Int((const ubyte*)(&offset), 0);
	ubyte* data = new ubyte[length];
	input.read((char*)data, length);
	input.close();

	return data;
}

// Записывает все измененные регионы на диск
void WorldFiles::write(const World* world, const Content* content){
	std::filesystem::path regions_dir = getRegionsFolder();

    if (!std::filesystem::is_directory(regions_dir)) {
		std::filesystem::create_directory(regions_dir);
	}

    writeWorldInfo(world);

    if (generatorTestMode) return;

	writeIndices(content->indices);

	for (auto& [key, region] : regions){
		if (region->getChunks() == nullptr || !region->isUnsaved()) continue;

		writeRegion(key.x, key.y, region, getRegionFile(key.x, key.y));
	}
}

void WorldFiles::writeWorldInfo(const World* world) {
	json::JObject root;

	json::JObject& versionobj = root.putObj("version");
	versionobj.put("major", ENGINE_VERSION_MAJOR);
	versionobj.put("minor", ENGINE_VERSION_MINOR);
	versionobj.put("maintenance", ENGINE_VERSION_MAINTENANCE);

	root.put("name", world->name);
	root.put("seed", world->seed);
	
	json::JObject& timeobj = root.putObj("time");
	timeobj.put("day-time", world->daytime);
	timeobj.put("day-time-speed", world->daytimeSpeed);

	files::write_string(getWorldFile(), json::stringify(&root, true, "  "));
}

bool WorldFiles::readWorldInfo(World* world) {
	std::filesystem::path file = getWorldFile();
	if (!std::filesystem::is_regular_file(file)) {
		LOG_WARN("World.json does not exists");
		return false;
	}

	std::unique_ptr<json::JObject> root(files::read_json(file));
	root->num("seed", world->seed);

	json::JObject* verobj = root->obj("version");
	if (verobj) {
		int major = 0, minor = -1, maintenance = -1;
		verobj->num("major", major);
		verobj->num("minor", minor);
		verobj->num("maintenance", maintenance);
		LOG_DEBUG("World version: {}.{}.{}", major, minor, maintenance);
	}

	json::JObject* timeobj = root->obj("time");
	if (timeobj) {
		timeobj->num("day-time", world->daytime);
		timeobj->num("day-time-speed", world->daytimeSpeed);
	}

	return true;
}

// Записываем данные об игроке на диск
void WorldFiles::writePlayer(Player* player){
	glm::vec3 position = player->hitbox->position;
	json::JObject root;
	json::JArray& posarr = root.putArray("position");
	posarr.put(position.x);
	posarr.put(position.y);
	posarr.put(position.z);

	json::JArray& rotarr = root.putArray("rotation");
	rotarr.put(player->camX);
	rotarr.put(player->camY);
	
	root.put("flight", player->flight);
	root.put("noclip", player->noclip);

	files::write_string(getPlayerFile(), json::stringify(&root, true, "  "));
}

// Читаем данные об игроке с диска
bool WorldFiles::readPlayer(Player* player) {
	std::filesystem::path file = getPlayerFile();
	if (!std::filesystem::is_regular_file(file)) {
		LOG_WARN("Player.json does not exists");
		return false;
	}

	std::unique_ptr<json::JObject> root(files::read_json(file));
	json::JArray* posarr = root->arr("position");
	glm::vec3& position = player->hitbox->position;
	position.x = posarr->num(0);
	position.y = posarr->num(1);
	position.z = posarr->num(2);
	player->camera->position = position;

	json::JArray* rotarr = root->arr("rotation");
	player->camX = rotarr->num(0);
	player->camY = rotarr->num(1);

	root->flag("flight", player->flight);
	root->flag("noclip", player->noclip);
	return true;
}

void WorldFiles::writeIndices(const ContentIndices* indices) {
	json::JObject root;
	json::JArray& blocks = root.putArray("blocks");
	uint count = indices->countBlockDefs();
	for (uint i = 0; i < count; ++i) {
		const Block* def = indices->getBlockDef(i);
		blocks.put(def->name);
	}
	files::write_string(getIndicesFile(), json::stringify(&root, true, "  "));
}

void WorldFiles::writeRegion(int x, int y, WorldRegion* entry, std::filesystem::path filename){
	ubyte** region = entry->getChunks();
	uint32_t* sizes = entry->getSizes();
	for (size_t i = 0; i < RegionConsts::VOLUME; ++i) {
		int chunk_x = (i % RegionConsts::SIZE) + x * RegionConsts::SIZE;
		int chunk_z = (i / RegionConsts::SIZE) + y * RegionConsts::SIZE;
		if (region[i] == nullptr) {
			region[i] = readChunkData(chunk_x, chunk_z, sizes[i], filename);
		}
	}

	char header[13] = REGION_FORMAT_MAGIC;
    header[11] = REGION_FORMAT_VERSION;
    header[12] = 0;

	std::ofstream file(filename, std::ios::out | std::ios::binary);
	file.write(header, 10);

	size_t offset = 10;
	char intbuf[4]{};
	uint offsets[RegionConsts::VOLUME]{};
	
	for (size_t i = 0; i < RegionConsts::VOLUME; i++) {
		ubyte* chunk = region[i];
		if (chunk == nullptr){
			offsets[i] = 0;
		} else {
			offsets[i] = offset;

			size_t compressedSize = sizes[i];
			int2Bytes(compressedSize, (ubyte*)intbuf, 0);
			offset += 4 + compressedSize;

			file.write(intbuf, 4);
			file.write((const char*)chunk, compressedSize);
		}
	}
	for (size_t i = 0; i < RegionConsts::VOLUME; ++i) {
		int2Bytes(offsets[i], (ubyte*)intbuf, 0);
		file.write(intbuf, 4);
	}
}
