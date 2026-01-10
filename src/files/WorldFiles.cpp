#include "WorldFiles.h"

#include <cassert>
#include <iostream>
#include <cstdint>
#include <fstream>

#include "files.h"
#include "../voxels/Chunk.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../window/Camera.h"
#include "../logger/Logger.h"

union {
	long _key;
	int _coords[2]; // x z
} _tempcoords;

// Общий объём сжатых файлов
ulong WorldFiles::totalCompressed = 0;

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
inline int bytes2Int(const ubyte* src, uint offset){
	return (src[offset] << 24) | (src[offset + 1] << 16) | (src[offset + 2] << 8) | (src[offset + 3]);
}

// Конвертирует целое число в 4 байта
inline void int2Bytes(int value, char* dest, uint offset){
	dest[offset] = (char) ((value >> 24) & 0xFF);
	dest[offset + 1] = (char) ((value >> 16) & 0xFF);
	dest[offset + 2] = (char) ((value >> 8) & 0xFF);
	dest[offset + 3] = (char) ((value >> 0) & 0xFF);
}

inline void float2Bytes(float fvalue, char* dest, uint offset){
	uint32_t value = *((uint32_t*)&fvalue);
	dest[offset] = (char) ((value >> 24) & 0xFF);
	dest[offset + 1] = (char) ((value >> 16) & 0xFF);
	dest[offset + 2] = (char) ((value >> 8) & 0xFF);
	dest[offset + 3] = (char) ((value >> 0) & 0xFF);
}

inline float bytes2Float(char* srcs, uint offset){
	ubyte* src = (ubyte*) srcs;
	uint32_t value = ((src[offset] << 24) |
					(src[offset + 1] << 16) |
					(src[offset + 2] << 8) |
					(src[offset + 3]));
	return *(float*)(&value);
}

// Конструктор
WorldFiles::WorldFiles(std::string directory, size_t mainBufferCapacity) : directory(directory){
    // Проверяем существование директории. Если её нет, то пытаемся создать
    if (!ensureDirectoryExists(directory)) {
        // Найти или создать директорию не удалось.
        LOG_CRITICAL("Failed to load world directory");
        throw std::runtime_error("Failed to load world directory");
    }

    // Инициализируем буферы
	mainBufferIn = new char[CHUNK_VOLUME * 2];
	mainBufferOut = new char[mainBufferCapacity];
}

// Деструктор 
WorldFiles::~WorldFiles(){
    // Освобождаем буферы
	delete[] mainBufferIn;
	delete[] mainBufferOut;

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
void WorldFiles::put(const char* chunkData, int x, int z){
	assert(chunkData != nullptr);

    // Вычисляем координаты региона
	int regionX = x >> Region_Consts::REGION_SIZE_BIT;
	int regionZ = z >> Region_Consts::REGION_SIZE_BIT;

    // Локальные координаты внутри региона
	int localX = x - (regionX << Region_Consts::REGION_SIZE_BIT);
	int localZ = z - (regionZ << Region_Consts::REGION_SIZE_BIT);

    // Создаем ключ для региона
	_tempcoords._coords[0] = regionX;
	_tempcoords._coords[1] = regionZ;
    long regionKey = _tempcoords._key;

    // Получаем или создаем регион
	WorldRegion& region = regions[regionKey];
    region.unsaved = true;
	if (region.chunksData == nullptr){
		region.chunksData = new char*[Region_Consts::REGION_VOLUME];
        for (uint i = 0; i < CHUNK_VOLUME; ++i) {
            region.chunksData[i] = nullptr;
        }
	}

    // Получаем указатель на чанк в регионе
    int target_index = localZ * Region_Consts::REGION_SIZE + localX;
	char* targetChunk = region.chunksData[target_index];

    // Если чанк не существует, создаем его
	if (targetChunk == nullptr){
		targetChunk = new char[CHUNK_VOLUME];
		region.chunksData[target_index] = targetChunk;
		totalCompressed += CHUNK_VOLUME;
	}

    // Копируем данные
	for (uint i = 0; i < CHUNK_VOLUME; ++i) {
		targetChunk[i] = chunkData[i];
    }
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
bool WorldFiles::getChunk(int x, int z, char* out){
	assert(out != nullptr);

    // Вычисляем координаты региона и локальные координаты
	int regionX = x >> Region_Consts::REGION_SIZE_BIT;
	int regionZ = z >> Region_Consts::REGION_SIZE_BIT;

	int localX = x - (regionX << Region_Consts::REGION_SIZE_BIT);
	int localZ = z - (regionZ << Region_Consts::REGION_SIZE_BIT);
	int chunk_index = localZ * Region_Consts::REGION_SIZE + localX;

	assert(chunk_index >= 0 && chunk_index < Region_Consts::REGION_VOLUME);

    // Создаем ключ региона
	_tempcoords._coords[0] = regionX;
	_tempcoords._coords[1] = regionZ;
    long region_key = _tempcoords._key;

    // Пытаемся получить из кэша
	WorldRegion& region = regions[region_key];
	if (region.chunksData == nullptr) return readChunk(x, z, out);

	char* chunk = region.chunksData[chunk_index];
	if (chunk == nullptr) return readChunk(x, z, out);

    // Копируем данные из кэша
	for (uint i = 0; i < CHUNK_VOLUME; ++i) {
		out[i] = chunk[i];
    }

	return true;
}

// Читает чанк непосредственно из файла
bool WorldFiles::readChunk(int x, int z, char* out){
	assert(out != nullptr);

    // Вычисляем координаты региона и локальные координаты
	int regionX = x >> Region_Consts::REGION_SIZE_BIT;
	int regionZ = z >> Region_Consts::REGION_SIZE_BIT;

	int localX = x - (regionX << Region_Consts::REGION_SIZE_BIT);
	int localZ = z - (regionZ << Region_Consts::REGION_SIZE_BIT);
	int chunk_index = localZ * Region_Consts::REGION_SIZE + localX;

    // Открываем файл
	std::string filename = getRegionFile(regionX, regionZ);

	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()) return false;

    // Читаем смещение чанка в файле
	uint32_t offset;
	input.seekg(chunk_index * sizeof(uint32_t));
	input.read((char*)(&offset), sizeof(uint32_t));

	offset = bytes2Int((const ubyte*)(&offset), 0); // Конвертируем

    // Проверяем валидность смещения
	if (offset == 0){
		input.close();
		return false;
	}

    // Читаем размер сжатых данных
	input.seekg(offset);
	input.read((char*)(&offset), sizeof(uint32_t));
	size_t compressedSize = bytes2Int((const ubyte*)(&offset), 0);

    // Читаем сжатые данные
	input.read(mainBufferIn, compressedSize);
	input.close();

    // Распаковываем
	decompressRLE((ubyte*)mainBufferIn, compressedSize, (ubyte*)out, CHUNK_VOLUME);

	return true;
}

// Записывает все измененные регионы на диск
void WorldFiles::write(){
	for (auto& [key, region] : regions){
		if (region.chunksData == nullptr || !region.unsaved) continue;

		int x, z;
		longToCoords(x, z, key);

		uint size = writeRegion(mainBufferOut, x, z, region.chunksData);
		write_binary_file(getRegionFile(x, z), mainBufferOut, size);
	}
}

// Записываем данные об игроке на диск
void WorldFiles::writePlayer(Player* player){
	char dst[1+3*sizeof(float) + 1+2*sizeof(float) + 1+1];

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

	write_binary_file(getPlayerFile(), (const char*)dst, sizeof(dst));
}

// Читаем данные об игроке с диска
bool WorldFiles::readPlayer(Player* player) {
	size_t length = 0;
	char* data = read_binary_file(getPlayerFile(), length);
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
            ubyte flags = data[offset++];
			player->flight = flags & Player_Flags::FLIGHT;
			player->noclip = flags & Player_Flags::NOCLIP;
		}
	}
	player->hitbox->position = position;
	player->camera->position = position + glm::vec3(0, 1, 0);

	return true;
}

// Формирует бинарное представление региона для записи
uint WorldFiles::writeRegion(char* out, int x, int z, char** region){
	uint offset = Region_Consts::REGION_VOLUME * sizeof(uint32_t);
	std::fill_n(out, offset, 0);

	char* compressed = new char[CHUNK_VOLUME * 2];  // Временный буфер для сжатия

	for (int i = 0; i < Region_Consts::REGION_VOLUME; ++i){
		char* chunk = region[i];

        // Если чанк отсутствует в памяти, пытаемся загрузить из файла
		if (chunk == nullptr){
			chunk = new char[CHUNK_VOLUME];

			assert((((i % Region_Consts::REGION_SIZE) + x * Region_Consts::REGION_SIZE) >> Region_Consts::REGION_SIZE_BIT) == x);
			assert((((i / Region_Consts::REGION_SIZE) + z * Region_Consts::REGION_SIZE) >> Region_Consts::REGION_SIZE_BIT) == z);
			
            if (readChunk((i % Region_Consts::REGION_SIZE) + x * Region_Consts::REGION_SIZE, (i / Region_Consts::REGION_SIZE) + z * Region_Consts::REGION_SIZE, chunk)){
				region[i] = chunk;
				totalCompressed += CHUNK_VOLUME;
			} else {
				delete[] chunk;
				chunk = nullptr;
			}
		}

		if (chunk == nullptr){
            // Чанк не существует
			int2Bytes(0, out, i * sizeof(uint32_t));
		} else {
            // Сохраняем смещение чанка
			int2Bytes(offset, out, i * sizeof(uint32_t));

            // Сжимаем данные чанка
			uint compressedSize = compressRLE((ubyte*)chunk, CHUNK_VOLUME, (ubyte*)compressed);

            // Записываем размер сжатых данных
			int2Bytes(compressedSize, out, offset);
			offset += sizeof(uint32_t);

            // Копируем сжатые данные
			for (uint j = 0; j < compressedSize; j++) {
				out[offset++] = compressed[j];
            }
		}
	}
	delete[] compressed;
	return offset;
}

// Конвертирует ключ региона в координаты
void longToCoords(int& x, int& z, long key) {
	_tempcoords._key = key;
	x = _tempcoords._coords[0];
	z = _tempcoords._coords[1];
}
