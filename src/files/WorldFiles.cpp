#include "WorldFiles.h"

#include <cassert>
#include <string>
#include <cstdint>
#include <memory>
#include <fstream>

#include "files.h"
#include "rle.h"
#include "binary_io.h"
#include "../window/Camera.h"
#include "../content/Content.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"
#include "../voxels/Chunk.h"
#include "../typedefs.h"
#include "../math/voxmaths.h"
#include "../world/World.h"
#include "../lighting/LightMap.h"
#include "../logger/Logger.h"
#include "../util/data_io.h"
#include "../coders/json.h"
#include "../constants.h"

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

WorldFiles::WorldFiles(std::filesystem::path directory, const DebugSettings& settings) : directory(directory), generatorTestMode(settings.generatorTestMode), doWriteLights(settings.doWriteLights) {
	compressionBuffer = new ubyte[CHUNK_DATA_LEN * 2];
}

WorldFiles::~WorldFiles(){
	delete[] compressionBuffer;
	for (auto it : regions){
		delete it.second;
	}
	regions.clear();
}

WorldRegion* WorldFiles::getRegion(std::unordered_map<glm::ivec2, WorldRegion*>& regions, int x, int z) {
	auto found = regions.find(glm::ivec2(x, z));
	if (found == regions.end()) return nullptr;
	return found->second;
}

WorldRegion* WorldFiles::getOrCreateRegion(std::unordered_map<glm::ivec2, WorldRegion*>& regions, int x, int z) {
	WorldRegion* region = getRegion(regions, x, z);
	if (region == nullptr) {
		region = new WorldRegion();
		regions[glm::ivec2(x, z)] = region;
	}
	return region;
}

ubyte* WorldFiles::compress(const ubyte* src, size_t srclen, size_t& len) {
	len = extrle::encode(src, srclen, compressionBuffer);
	ubyte* data = new ubyte[len];
	for (size_t i = 0; i < len; ++i) {
		data[i] = compressionBuffer[i];
	}
	return data;
}

ubyte* WorldFiles::decompress(const ubyte* src, size_t srclen, size_t dstlen) {
	ubyte* decompressed = new ubyte[dstlen];
	extrle::decode(src, srclen, decompressed);
	return decompressed;
}

void WorldFiles::put(Chunk* chunk){
	assert(chunk != nullptr);

	int regionX = floordiv(chunk->chunk_x, RegionConsts::SIZE);
	int regionZ = floordiv(chunk->chunk_z, RegionConsts::SIZE);

	int localX = chunk->chunk_x - (regionX * RegionConsts::SIZE);
	int localZ = chunk->chunk_z - (regionZ * RegionConsts::SIZE);

	{
		WorldRegion* region = getOrCreateRegion(regions, regionX, regionZ);
		region->setUnsaved(true);
		std::unique_ptr<ubyte[]> chunk_data (chunk->encode());
		size_t compressedSize;
		ubyte* data = compress(chunk_data.get(), CHUNK_DATA_LEN, compressedSize);
		region->put(localX, localZ, data, compressedSize);
	}
	if (doWriteLights && chunk->isLighted()) {
		WorldRegion* region = getOrCreateRegion(lights, regionX, regionZ);
		region->setUnsaved(true);
		std::unique_ptr<ubyte[]> light_data (chunk->light_map->encode());
		size_t compressedSize;
		ubyte* data = compress(light_data.get(), LIGHTMAP_DATA_LEN, compressedSize);
		region->put(localX, localZ, data, compressedSize);
	}
}

void WorldFiles::put(int x, int z, const ubyte* voxelData) {
    int regionX = floordiv(x, RegionConsts::SIZE);
	int regionZ = floordiv(z, RegionConsts::SIZE);

	int localX = x - (regionX * RegionConsts::SIZE);
	int localZ = z - (regionZ * RegionConsts::SIZE);

	{
		WorldRegion* region = getOrCreateRegion(regions, regionX, regionZ);
		region->setUnsaved(true);
		size_t compressedSize;
		ubyte* data = compress(voxelData, CHUNK_DATA_LEN, compressedSize);
		region->put(localX, localZ, data, compressedSize);
	}
}

std::filesystem::path WorldFiles::getRegionsFolder() const {
	return directory/std::filesystem::path("regions");
}

std::filesystem::path WorldFiles::getLightsFolder() const {
	return directory/std::filesystem::path("lights");
}

std::filesystem::path WorldFiles::getRegionFilename(int x, int y) const {
	std::string filename = std::to_string(x) + "_" + std::to_string(y) + ".bin";
	return std::filesystem::path(filename);
}

bool WorldFiles::parseRegionFilename(const std::string& name, int& x, int& y) {
    size_t sep = name.find('_');

    if (sep == std::string::npos || sep == 0 || sep == name.length() - 1) return false;

    try {
        x = std::stoi(name.substr(0, sep));
        y = std::stoi(name.substr(sep + 1));
    } catch (std::invalid_argument& err) {
        return false;
    } catch (std::out_of_range& err) {
        return false;
    }

    return true;
}

std::filesystem::path WorldFiles::getPlayerFile() const {
	return directory/std::filesystem::path("player.json");
}

std::filesystem::path WorldFiles::getWorldFile() const {
	return directory/std::filesystem::path("world.json");
}

std::filesystem::path WorldFiles::getIndicesFile() const {
	return directory/std::filesystem::path("indices.json");
}

ubyte* WorldFiles::getChunk(int x, int z){
	return getData(regions, getRegionsFolder(), x, z);
}

light_t* WorldFiles::getLights(int x, int z) {
	ubyte* data = getData(lights, getLightsFolder(), x, z);
	if (data == nullptr) return nullptr;
	return LightMap::decode(data);
}

ubyte* WorldFiles::getData(std::unordered_map<glm::ivec2, WorldRegion*>& regions, const std::filesystem::path& folder, int x, int z) {
	int regionX = floordiv(x, RegionConsts::SIZE);
	int regionZ = floordiv(z, RegionConsts::SIZE);

	int localX = x - (regionX * RegionConsts::SIZE);
	int localZ = z - (regionZ * RegionConsts::SIZE);

	WorldRegion* region = getRegion(regions, regionX, regionZ);
	if (region == nullptr) {
		region = new WorldRegion();
		regions[glm::ivec2(regionX, regionZ)] = region;
	}

	ubyte* data = region->get(localX, localZ);
	if (data == nullptr) {
		uint32_t size;
		data = readChunkData(x, z, size, folder/getRegionFilename(regionX, regionZ));
		if (data) region->put(localX, localZ, data, size);
	}

	if (data) return decompress(data, region->getSize(localX, localZ), CHUNK_DATA_LEN);

	return nullptr;
}

ubyte* WorldFiles::readChunkData(int x, int z, uint32_t& length, std::filesystem::path filename){
	if (generatorTestMode) return nullptr;

	int regionX = floordiv(x, RegionConsts::SIZE);
	int regionZ = floordiv(z, RegionConsts::SIZE);

	int localX = x - (regionX * RegionConsts::SIZE);
	int localZ = z - (regionZ * RegionConsts::SIZE);

	int chunkIndex = localZ * RegionConsts::SIZE + localX;

	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open())return nullptr;

	input.seekg(0, std::ios::end);
	size_t file_size = input.tellg();
	size_t table_offset = file_size - RegionConsts::VOLUME * 4;

	uint32_t offset;
	input.seekg(table_offset + chunkIndex * 4);
	input.read((char*)(&offset), 4);
	offset = dataio::read_int32_big((const ubyte*)(&offset), 0);
	if (offset == 0){
		input.close();
		return nullptr;
	}
	input.seekg(offset);
	input.read((char*)(&offset), 4);
	length = dataio::read_int32_big((const ubyte*)(&offset), 0);
	ubyte* data = new ubyte[length];
	input.read((char*)data, length);
	input.close();
	return data;
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
			dataio::write_int32_big(compressedSize, (ubyte*)intbuf, 0);
			offset += 4 + compressedSize;

			file.write(intbuf, 4);
			file.write((const char*)chunk, compressedSize);
		}
	}
	for (size_t i = 0; i < RegionConsts::VOLUME; i++) {
		dataio::write_int32_big(offsets[i], (ubyte*)intbuf, 0);
		file.write(intbuf, 4);
	}
}

void WorldFiles::writeRegions(std::unordered_map<glm::ivec2, WorldRegion*>& regions, const std::filesystem::path& folder) {
	for (auto it : regions){
		WorldRegion* region = it.second;
		if (region->getChunks() == nullptr || !region->isUnsaved()) continue;
		glm::ivec2 key = it.first;
		writeRegion(key.x, key.y, region, folder/getRegionFilename(key.x, key.y));
	}
}

void WorldFiles::write(const World* world, const Content* content) {
	{
		std::filesystem::path directory = getRegionsFolder();
		if (!std::filesystem::is_directory(directory)) std::filesystem::create_directories(directory);
	}
	{
		std::filesystem::path directory = getLightsFolder();
		if (!std::filesystem::is_directory(directory)) std::filesystem::create_directories(directory);
	}
	if (world) writeWorldInfo(world);
	if (generatorTestMode) return;
	writeIndices(content->indices);
	writeRegions(regions, getRegionsFolder());
	writeRegions(lights, getLightsFolder());
}

void WorldFiles::writeIndices(const ContentIndices* indices) {
	json::JObject root;
	json::JArray& blocks = root.putArray("blocks");
	uint count = indices->countBlockDefs();
	for (uint i = 0; i < count; i++) {
		const Block* def = indices->getBlockDef(i);
		blocks.put(def->name);
	}
	files::write_string(getIndicesFile(), json::stringify(&root, true, "  "));
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
	root->str("name", world->name);
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
