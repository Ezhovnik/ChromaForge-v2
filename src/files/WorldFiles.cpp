#include "WorldFiles.h"

#include <cassert>
#include <iostream>
#include <cstdint>
#include <fstream>

#include "files.h"
#include "rle.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../window/Camera.h"
#include "../math/voxmaths.h"
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

inline void float2Bytes(float fvalue, ubyte* dest, size_t offset){
	uint32_t value = *((uint32_t*)&fvalue);
	dest[offset] = (char) ((value >> 24) & 0xFF);
	dest[offset + 1] = (char) ((value >> 16) & 0xFF);
	dest[offset + 2] = (char) ((value >> 8) & 0xFF);
	dest[offset + 3] = (char) ((value >> 0) & 0xFF);
}

inline float bytes2Float(ubyte* src, uint offset){
	uint32_t value = ((src[offset] << 24) |
					(src[offset + 1] << 16) |
					(src[offset + 2] << 8) |
					(src[offset + 3]));
	return *(float*)(&value);
}

// Конструктор
WorldFiles::WorldFiles(std::string directory) : directory(directory){
    // Проверяем существование директории. Если её нет, то пытаемся создать
    if (!ensureDirectoryExists(directory)) {
        // Найти или создать директорию не удалось.
        LOG_CRITICAL("Failed to load world directory");
        throw std::runtime_error("Failed to load world directory");
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
        for (uint i = 0; i < RegionConsts::VOLUME; ++i){
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
	int regionX = floordiv(chunk->chunk_x, RegionConsts::SIZE);
    int regionZ = floordiv(chunk->chunk_z, RegionConsts::SIZE);

    // Локальные координаты внутри региона
	int localX = chunk->chunk_x - (regionX * RegionConsts::SIZE);
	int localZ = chunk->chunk_z - (regionZ * RegionConsts::SIZE);

    glm::ivec2 key(regionX, regionZ);

    auto found = regions.find(key);
    if (found == regions.end()) {
        ubyte** chunksData = new ubyte*[RegionConsts::VOLUME];
        uint32_t* compressedSizes = new uint32_t[RegionConsts::VOLUME];
        for (uint i = 0; i < RegionConsts::VOLUME; ++i) {
            chunksData[i] = nullptr;
        }
        regions[key] = {chunksData, compressedSizes, true};
    }

    // Получаем или создаем регион
	WorldRegion& region = regions[key];
    region.unsaved = true;

    size_t target_index = localZ * RegionConsts::SIZE + localX;
	ubyte* targetChunk = region.chunksData[target_index];
    if (targetChunk) {
        delete[] targetChunk;
    }

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
std::string WorldFiles::getRegionFile(int x, int z) {
	return directory + std::to_string(x) + "_" + std::to_string(z) + ".bin";
}

// Генерирует имя файла, в котором записана информация об игроке
std::string WorldFiles::getPlayerFile() {
	return directory + "/player.bin";
}

// Получает данные чанка из кэша или файла
ubyte* WorldFiles::getChunk(int x, int z){
	// Вычисляем координаты региона
	int regionX = floordiv(x, RegionConsts::SIZE);
    int regionZ = floordiv(z, RegionConsts::SIZE);

    // Локальные координаты внутри региона
	int localX = x - (regionX * RegionConsts::SIZE);
	int localZ = z - (regionZ * RegionConsts::SIZE);

	int chunk_index = localZ * RegionConsts::SIZE + localX;

	assert(chunk_index >= 0 && chunk_index < RegionConsts::VOLUME);

    // Создаем ключ региона
	glm::ivec2 key(regionX, regionZ);

    auto found = regions.find(key);
    if (found == regions.end()) {
        ubyte** chunksData = new ubyte* [RegionConsts::VOLUME];
        uint32_t* compressedSizes = new uint32_t[RegionConsts::VOLUME];
        for (uint i = 0; i < RegionConsts::VOLUME; ++i) {
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
    // Вычисляем координаты региона
	int regionX = floordiv(x, RegionConsts::SIZE);
    int regionZ = floordiv(z, RegionConsts::SIZE);

    // Локальные координаты внутри региона
	int localX = x - (regionX * RegionConsts::SIZE);
	int localZ = z - (regionZ * RegionConsts::SIZE);

	int chunk_index = localZ * RegionConsts::SIZE + localX;

    // Открываем файл
	std::string filename = getRegionFile(regionX, regionZ);

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
void WorldFiles::write(){
	for (auto& [key, region] : regions){
		if (region.chunksData == nullptr || !region.unsaved) continue;

		writeRegion(key.x, key.y, region);
	}
}

// Записываем данные об игроке на диск
void WorldFiles::writePlayer(Player* player){
	ubyte dst[1+3*sizeof(float) + 1+2*sizeof(float) + 1+1];

	glm::vec3 position = player->hitbox->position;

	size_t offset = 0;
	dst[offset++] = PlayerSections::POSITION;
	float2Bytes(position.x, dst, offset); offset += sizeof(float);
	float2Bytes(position.y, dst, offset); offset += sizeof(float);
	float2Bytes(position.z, dst, offset); offset += sizeof(float);

	dst[offset++] = PlayerSections::ROTATION;
	float2Bytes(player->camX, dst, offset); offset += sizeof(float);
	float2Bytes(player->camY, dst, offset); offset += sizeof(float);

    dst[offset++] = PlayerSections::FLAGS;
	dst[offset++] = player->flight * PlayerFlags::FLIGHT | player->noclip * PlayerFlags::NOCLIP;

	write_binary_file(getPlayerFile(), (const char*)dst, sizeof(dst));
}

// Читаем данные об игроке с диска
bool WorldFiles::readPlayer(Player* player) {
	size_t length = 0;
	ubyte* data = (ubyte*)read_binary_file(getPlayerFile(), length);
	if (data == nullptr){
        LOG_WARN("Could not to read player.bin (ignored)");
		return false;
	}

	glm::vec3 position = player->hitbox->position;
	size_t offset = 0;
	while (offset < length){
		char section = data[offset++];
		switch (section){
		case PlayerSections::POSITION:
			position.x = bytes2Float(data, offset); offset += sizeof(float);
			position.y = bytes2Float(data, offset); offset += sizeof(float);
			position.z = bytes2Float(data, offset); offset += sizeof(float);
			break;
		case PlayerSections::ROTATION:
			player->camX = bytes2Float(data, offset); offset += sizeof(float);
			player->camY = bytes2Float(data, offset); offset += sizeof(float);
			break;
        case PlayerSections::FLAGS:
            {
                ubyte flags = data[offset++];
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
    for (size_t i = 0; i < RegionConsts::VOLUME; ++i) {
        int chunk_x = (i % RegionConsts::SIZE) + x * RegionConsts::SIZE;
		int chunk_z = (i / RegionConsts::SIZE) + z * RegionConsts::SIZE;
		if (region[i] == nullptr) {
			region[i] = readChunkData(chunk_x, chunk_z, sizes[i]);
		}
    }

    char header[13] = ".CHROMAREG";
    header[11] = REGION_FORMAT_VERSION;
    header[12] = 0;

    std::ofstream file(getRegionFile(x, z), std::ios::out | std::ios::binary);
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
	for (size_t i = 0; i < RegionConsts::VOLUME; i++) {
		int2Bytes(offsets[i], (ubyte*)intbuf, 0);
		file.write(intbuf, 4);
	}
}
