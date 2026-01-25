#include "WorldFiles.h"

#include <cassert>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <filesystem>

#include "files.h"
#include "rle.h"
#include "binary_io.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../window/Camera.h"
#include "../math/voxmaths.h"
#include "../constants.h"
#include "../logger/Logger.h"

// Константы для идентификации секций в файлах
namespace PlayerSections {
    constexpr int POSITION = 1; // Секция с позицией игрока
    constexpr int ROTATION = 2; // Секция с поворотом камеры игрока
    constexpr int FLAGS = 3;
}

namespace PlayerFlags {
    constexpr int FLIGHT = 0x1;
    constexpr int NOCLIP = 0x2;
}

namespace WorldSections {
    constexpr int MAIN = 1;
}

// Конвертирует 4 байта в целое число
inline int bytes2Int(const ubyte* src, size_t offset){
	return (src[offset] << 24) | (src[offset + 1] << 16) | (src[offset + 2] << 8) | (src[offset + 3]);
}

// Конвертирует целое число в 4 байта
inline void int2Bytes(int value, ubyte* dest, size_t offset){
	dest[offset] = (char) (value >> 24 & 0xFF);
	dest[offset + 1] = (char) (value >> 16 & 0xFF);
	dest[offset + 2] = (char) (value >> 8 & 0xFF);
	dest[offset + 3] = (char) (value >> 0 & 0xFF);
}

// Конструктор
WorldFiles::WorldFiles(std::filesystem::path directory, bool generatorTestMode) : directory(directory), generatorTestMode(generatorTestMode){
    // Проверяем существование директории. Если её нет, то пытаемся создать
    if (!std::filesystem::is_directory(directory)) {
		std::filesystem::create_directories(directory);
	}

    // Инициализируем буферы
	compressionBuffer = new ubyte[CHUNK_DATA_LEN * 2];
}

// Деструктор 
WorldFiles::~WorldFiles(){
    // Освобождаем буферы
	delete[] compressionBuffer;

    // Осовбождаем регионы из памяти
	for (auto& [key, region] : regions){
        if (region.chunksData == nullptr) continue;
        for (uint i = 0; i < RegionConsts::REGION_VOLUME; ++i){
            delete[] region.chunksData[i];
        }
        delete[] region.chunksData;
	}
	regions.clear();
}

// Помещает данные чанка в кэш памяти
void WorldFiles::put(Chunk* chunk){
	assert(chunk != nullptr);

    // Вычисляем координаты региона
	int regionX = floordiv(chunk->chunk_x, RegionConsts::REGION_SIZE);
    int regionZ = floordiv(chunk->chunk_z, RegionConsts::REGION_SIZE);

    // Локальные координаты внутри региона
	int localX = chunk->chunk_x - (regionX * RegionConsts::REGION_SIZE);
	int localZ = chunk->chunk_z - (regionZ * RegionConsts::REGION_SIZE);

    glm::ivec2 key(regionX, regionZ);

    auto found = regions.find(key);
    if (found == regions.end()) {
        ubyte** chunksData = new ubyte*[RegionConsts::REGION_VOLUME];
        uint32_t* compressedSizes = new uint32_t[RegionConsts::REGION_VOLUME];
        for (uint i = 0; i < RegionConsts::REGION_VOLUME; ++i) {
            chunksData[i] = nullptr;
        }
        regions[key] = {chunksData, compressedSizes, true};
    }

    // Получаем или создаем регион
	WorldRegion& region = regions[key];
    region.unsaved = true;

    size_t target_index = localZ * RegionConsts::REGION_SIZE + localX;
	ubyte* targetChunk = region.chunksData[target_index];
    if (targetChunk) delete[] targetChunk;

    ubyte* chunk_data = chunk->encode();
    size_t compressedSize = extrle::encode(chunk_data, CHUNK_DATA_LEN, compressionBuffer);
    delete[] chunk_data;
    ubyte* data = new ubyte[compressedSize];
    for (size_t i = 0; i < compressedSize; ++i) {
        data[i] = compressionBuffer[i];
    }

    region.chunksData[target_index] = data;
    region.compressedSizes[target_index] = compressedSize;
}

// Генерирует имя файла для региона с заданными координатами
std::filesystem::path WorldFiles::getRegionFile(int x, int z) const {
	return directory/std::filesystem::path(std::to_string(x) + "_" + std::to_string(z) + ".bin");
}

// Генерирует имя файла, в котором записана информация об игроке
std::filesystem::path WorldFiles::getPlayerFile() const {
	return directory/std::filesystem::path("player.bin");
}

std::filesystem::path WorldFiles::getWorldFile() const {
    return directory/std::filesystem::path("world.bin");
}

// Получает данные чанка из кэша или файла
ubyte* WorldFiles::getChunk(int x, int z){
	// Вычисляем координаты региона
	int regionX = floordiv(x, RegionConsts::REGION_SIZE);
    int regionZ = floordiv(z, RegionConsts::REGION_SIZE);

    // Локальные координаты внутри региона
	int localX = x - (regionX * RegionConsts::REGION_SIZE);
	int localZ = z - (regionZ * RegionConsts::REGION_SIZE);

	int chunk_index = localZ * RegionConsts::REGION_SIZE + localX;
	assert(chunk_index >= 0 && chunk_index < RegionConsts::REGION_VOLUME);

    // Создаем ключ региона
	glm::ivec2 key(regionX, regionZ);

    auto found = regions.find(key);
    if (found == regions.end()) {
        ubyte** chunksData = new ubyte * [RegionConsts::REGION_VOLUME];
        uint32_t* compressedSizes = new uint32_t[RegionConsts::REGION_VOLUME];
        for (uint i = 0; i < RegionConsts::REGION_VOLUME; ++i) {
            chunksData[i] = nullptr;
        }
        regions[key] = {chunksData, compressedSizes, true};
    }

    // Пытаемся получить из кэша
	WorldRegion& region = regions[key];
    ubyte* data = region.chunksData[chunk_index];
	if (data == nullptr) {
        data = readChunkData(x, z, region.compressedSizes[chunk_index]);
		if (data) region.chunksData[chunk_index] = data;
    }

    if (data) {
		ubyte* decompressed = new ubyte[CHUNK_DATA_LEN];
		extrle::decode(data, region.compressedSizes[chunk_index], decompressed);
		return decompressed;
	}
	return nullptr;
}

// Читает чанк непосредственно из файла
ubyte* WorldFiles::readChunkData(int x, int z, uint32_t& length){
    if (generatorTestMode) return nullptr;

	int regionX = floordiv(x, RegionConsts::REGION_SIZE);
    int regionZ = floordiv(z, RegionConsts::REGION_SIZE);

    // Локальные координаты внутри региона
	int localX = x - (regionX * RegionConsts::REGION_SIZE);
	int localZ = z - (regionZ * RegionConsts::REGION_SIZE);

	int chunk_index = localZ * RegionConsts::REGION_SIZE + localX;

    // Открываем файл
	std::filesystem::path filename = getRegionFile(regionX, regionZ);

	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()) return nullptr;

    input.seekg(0, std::ios::end);
	size_t file_size = input.tellg();
    size_t table_offset = file_size - RegionConsts::REGION_VOLUME * 4;

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
void WorldFiles::write(const WorldInfo info){
    if (!std::filesystem::is_directory(directory)) {
		std::filesystem::create_directory(directory);
	}

    writeWorldInfo(info);

    if (generatorTestMode) return;

	for (auto& [key, region] : regions){
		if (region.chunksData == nullptr || !region.unsaved) continue;

		writeRegion(key.x, key.y, region);
	}
}

void WorldFiles::writeWorldInfo(const WorldInfo& info) {
    BinaryWriter out;
    out.putCStr(WORLD_FORMAT_MAGIC);
    out.put(WORLD_FORMAT_VERSION);
    out.put((ubyte)WorldSections::MAIN);

    out.putInt64(info.seed);
    out.put(info.name);

    files::write_bytes(getWorldFile(), (const char*)out.data(), out.size());
}

bool WorldFiles::readWorldInfo(WorldInfo& info) {
	size_t length = 0;
	ubyte* data = (ubyte*)files::read_bytes(getWorldFile(), length);
	if (data == nullptr) {
        LOG_WARN("Could not to read world.bin (ignored)");
		return false;
	}
	BinaryReader inp(data, length);
	inp.checkMagic(WORLD_FORMAT_MAGIC, 11);
	/*ubyte version = */inp.get();
	while (inp.hasNext()) {
		ubyte section = inp.get();
		switch (section) {
		case WorldSections::MAIN:
			info.seed = inp.getInt64();
			info.name = inp.getString();
			break;
		}
	}
	return false;
}

// Записываем данные об игроке на диск
void WorldFiles::writePlayer(Player* player){
	glm::vec3 position = player->hitbox->position;

	BinaryWriter out;
	out.put(PlayerSections::POSITION);
	out.putFloat32(position.x);
	out.putFloat32(position.y);
	out.putFloat32(position.z);

	out.put(PlayerSections::ROTATION);
	out.putFloat32(player->camX);
    out.putFloat32(player->camY);

    out.put(PlayerSections::FLAGS);
	out.put(player->flight * PlayerFlags::FLIGHT | player->noclip * PlayerFlags::NOCLIP);

	files::write_bytes(getPlayerFile(), (const char*)out.data(), out.size());
}

// Читаем данные об игроке с диска
bool WorldFiles::readPlayer(Player* player) {
	size_t length = 0;
	ubyte* data = (ubyte*)files::read_bytes(getPlayerFile(), length);
	if (data == nullptr){
		LOG_WARN("Could not to read player.bin (ignored)");
		return false;
	}
	glm::vec3 position = player->hitbox->position;
	BinaryReader inp(data, length);
	while (inp.hasNext()) {
		ubyte section = inp.get();
		switch (section) {
		case PlayerSections::POSITION:
			position.x = inp.getFloat32();
			position.y = inp.getFloat32();
			position.z = inp.getFloat32();
			break;
		case PlayerSections::ROTATION:
			player->camX = inp.getFloat32();
			player->camY = inp.getFloat32();
			break;
		case PlayerSections::FLAGS: 
			{
				ubyte flags = inp.get();
				player->flight = flags & PlayerFlags::FLIGHT;
				player->noclip = flags & PlayerFlags::NOCLIP;
			}
			break;
		}
	}

	player->hitbox->position = position;
	player->camera->position = position + glm::vec3(0, 1, 0);
	return true;
}

// Формирует бинарное представление региона для записи
void WorldFiles::writeRegion(int x, int z, WorldRegion& entry){
	ubyte** region = entry.chunksData;
    uint32_t* sizes = entry.compressedSizes;

    for (size_t i = 0; i < RegionConsts::REGION_VOLUME; ++i) {
        int chunk_x = (i % RegionConsts::REGION_SIZE) + x * RegionConsts::REGION_SIZE;
		int chunk_y = (i / RegionConsts::REGION_SIZE) + z * RegionConsts::REGION_SIZE;
		if (region[i] == nullptr) region[i] = readChunkData(chunk_x, chunk_y, sizes[i]);
    }

    char header[13] = REGION_FORMAT_MAGIC;
    header[11] = REGION_FORMAT_VERSION;
    header[12] = 0; // Флаги

    std::ofstream file(getRegionFile(x, z), std::ios::out | std::ios::binary);
	file.write(header, 13);

    size_t offset = 13;
	char intbuf[4]{};
	uint offsets[RegionConsts::REGION_VOLUME]{};

    for (size_t i = 0; i < RegionConsts::REGION_VOLUME; ++i) {
        ubyte* chunk = region[i];
        if (chunk == nullptr) {
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
    for (size_t i = 0; i < RegionConsts::REGION_VOLUME; ++i) {
        int2Bytes(offsets[i], (ubyte*)intbuf, 0);
        file.write(intbuf, 4);
    }
}
