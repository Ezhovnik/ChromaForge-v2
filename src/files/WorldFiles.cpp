#include "WorldFiles.h"

#include <cassert>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <filesystem>

#include "files.h"
#include "rle.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../window/Camera.h"
#include "../math/voxmaths.h"
#include "../constants.h"
#include "../logger/Logger.h"

// Константы для идентификации секций в файлах
namespace Sections {
    constexpr int POSITION = 1; // Секция с позицией игрока
    constexpr int ROTATION = 2; // Секция с поворотом камеры игрока
    constexpr int FLAGS = 3;
}

namespace Player_Flags {
    constexpr int FLIGHT = 0x1;
    constexpr int NOCLIP = 0x2;
}

// Статические проверки размеров типов данных
static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes");
static_assert(sizeof(float) == 4, "float must be 4 bytes");
static_assert(sizeof(char) == 1, "char must be 1 byte");

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

inline void float2Bytes(float fvalue, ubyte* dest, size_t offset){
	uint32_t value = *((uint32_t*)&fvalue);
	dest[offset] = (char) (value >> 24 & 0xFF);
	dest[offset + 1] = (char) (value >> 16 & 0xFF);
	dest[offset + 2] = (char) (value >> 8 & 0xFF);
	dest[offset + 3] = (char) (value >> 0 & 0xFF);
}

inline float bytes2Float(ubyte* src, uint offset){
	uint32_t value = ((src[offset] << 24) |
					(src[offset + 1] << 16) |
					(src[offset + 2] << 8) |
					(src[offset + 3]));
	return *(float*)(&value);
}

// Конструктор
WorldFiles::WorldFiles(std::string directory, size_t mainBufferCapacity, bool generatorTestMode) : directory(directory), generatorTestMode(generatorTestMode){
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
	for (auto& [key, region] : regions){
        if (region.chunksData == nullptr) continue;
        for (uint i = 0; i < Region_Consts::REGION_VOLUME; ++i){
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
	int regionX = floordiv(chunk->chunk_x, Region_Consts::REGION_SIZE);
    int regionZ = floordiv(chunk->chunk_z, Region_Consts::REGION_SIZE);

    // Локальные координаты внутри региона
	int localX = chunk->chunk_x - (regionX * Region_Consts::REGION_SIZE);
	int localZ = chunk->chunk_z - (regionZ * Region_Consts::REGION_SIZE);

    glm::ivec2 key(regionX, regionZ);

    auto found = regions.find(key);
    if (found == regions.end()) {
        ubyte** chunksData = new ubyte*[Region_Consts::REGION_VOLUME];
        uint32_t* compressedSizes = new uint32_t[Region_Consts::REGION_VOLUME];
        for (uint i = 0; i < Region_Consts::REGION_VOLUME; ++i) {
            chunksData[i] = nullptr;
        }
        regions[key] = {chunksData, compressedSizes, true};
    }

    // Получаем или создаем регион
	WorldRegion& region = regions[key];
    region.unsaved = true;

    size_t target_index = localZ * Region_Consts::REGION_SIZE + localX;
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
	int regionX = floordiv(x, Region_Consts::REGION_SIZE);
    int regionZ = floordiv(z, Region_Consts::REGION_SIZE);

    // Локальные координаты внутри региона
	int localX = x - (regionX * Region_Consts::REGION_SIZE);
	int localZ = z - (regionZ * Region_Consts::REGION_SIZE);

	int chunk_index = localZ * Region_Consts::REGION_SIZE + localX;
	assert(chunk_index >= 0 && chunk_index < Region_Consts::REGION_VOLUME);

    // Создаем ключ региона
	glm::ivec2 key(regionX, regionZ);

    auto found = regions.find(key);
    if (found == regions.end()) {
        ubyte** chunksData = new ubyte * [Region_Consts::REGION_VOLUME];
        uint32_t* compressedSizes = new uint32_t[Region_Consts::REGION_VOLUME];
        for (uint i = 0; i < Region_Consts::REGION_VOLUME; ++i) {
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

	int regionX = floordiv(x, Region_Consts::REGION_SIZE);
    int regionZ = floordiv(z, Region_Consts::REGION_SIZE);

    // Локальные координаты внутри региона
	int localX = x - (regionX * Region_Consts::REGION_SIZE);
	int localZ = z - (regionZ * Region_Consts::REGION_SIZE);

	int chunk_index = localZ * Region_Consts::REGION_SIZE + localX;

    // Открываем файл
	std::string filename = getRegionFile(regionX, regionZ);

	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()) return nullptr;

    input.seekg(0, std::ios::end);
	size_t file_size = input.tellg();
    size_t table_offset = file_size - Region_Consts::REGION_VOLUME * 4;

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
    if (generatorTestMode) return;
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
	dst[offset++] = Sections::POSITION;
	float2Bytes(position.x, dst, offset); offset += sizeof(float);
	float2Bytes(position.y, dst, offset); offset += sizeof(float);
	float2Bytes(position.z, dst, offset); offset += sizeof(float);

	dst[offset++] = Sections::ROTATION;
	float2Bytes(player->camX, dst, offset); offset += sizeof(float);
	float2Bytes(player->camY, dst, offset); offset += sizeof(float);

    dst[offset++] = Sections::FLAGS;
	dst[offset++] = player->flight * Player_Flags::FLIGHT | player->noclip * Player_Flags::NOCLIP;

	files::write_bytes(getPlayerFile(), (const char*)dst, sizeof(dst));
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
	size_t offset = 0;
	while (offset < length){
		char section = data[offset++];
		switch (section){
		case Sections::POSITION:
			position.x = bytes2Float(data, offset); offset += sizeof(float);
			position.y = bytes2Float(data, offset); offset += sizeof(float);
			position.z = bytes2Float(data, offset); offset += sizeof(float);
			break;
		case Sections::ROTATION:
			player->camX = bytes2Float(data, offset); offset += sizeof(float);
			player->camY = bytes2Float(data, offset); offset += sizeof(float);
			break;
        case Sections::FLAGS:
            {
                ubyte flags = data[offset++];
                player->flight = flags & Player_Flags::FLIGHT;
                player->noclip = flags & Player_Flags::NOCLIP;
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

    for (size_t i = 0; i < Region_Consts::REGION_VOLUME; ++i) {
        int chunk_x = (i % Region_Consts::REGION_SIZE) + x * Region_Consts::REGION_SIZE;
		int chunk_y = (i / Region_Consts::REGION_SIZE) + z * Region_Consts::REGION_SIZE;
		if (region[i] == nullptr) region[i] = readChunkData(chunk_x, chunk_y, sizes[i]);
    }

    char header[13] = ".CHROMAREG";
    header[11] = REGION_FORMAT_VERSION;
    header[12] = 0; // Флаги

    std::ofstream file(getRegionFile(x, z), std::ios::out | std::ios::binary);
	file.write(header, 13);

    size_t offset = 13;
	char intbuf[4]{};
	uint offsets[Region_Consts::REGION_VOLUME]{};

    for (size_t i = 0; i < Region_Consts::REGION_VOLUME; ++i) {
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
    for (size_t i = 0; i < Region_Consts::REGION_VOLUME; ++i) {
        int2Bytes(offsets[i], (ubyte*)intbuf, 0);
        file.write(intbuf, 4);
    }
}
