#include "WorldFiles.h"

#include <cassert>
#include <iostream>
#include <cstdint>
#include <fstream>

#include "files.h"
#include "../voxels/Chunk.h"

union {
	long _key;
	int _coords[2];
} _tempcoords;

unsigned long WorldFiles::totalCompressed = 0;

// Конвертирует 4 байта в целое число
int bytes2Int(const unsigned char* src, uint offset){
	return (src[offset] << 24) | (src[offset + 1] << 16) | (src[offset + 2] << 8) | (src[offset + 3]);
}

// Конвертирует целое число в 4 байта
void int2Bytes(int value, char* dest, uint offset){
	dest[offset] = (char)((value >> 24) & 0xFF);
	dest[offset + 1] = (char)((value >> 16) & 0xFF);
	dest[offset + 2] = (char)((value >> 8) & 0xFF);
	dest[offset + 3] = (char)(value & 0xFF);
}

// Конструктор
WorldFiles::WorldFiles(const char* directory, size_t mainBufferCapacity) : directory(directory){
    if (!ensureDirectoryExists(directory)) {
        throw std::runtime_error("Failed to load world directory");
    }

	mainBufferIn = new char[CHUNK_VOLUME * 2];
	mainBufferOut = new char[mainBufferCapacity];
}

// Деструктор 
WorldFiles::~WorldFiles(){
	delete[] mainBufferIn;
	delete[] mainBufferOut;

	for (auto& [key, region] : regions){
        if (region == nullptr) continue;
        for (uint i = 0; i < REGION_VOLUME; i++){
            delete[] region[i];
        }
        delete[] region;
	}
	regions.clear();
}

// Помещает данные чанка в кэш памяти
void WorldFiles::put(const char* chunkData, int x, int z){
	assert(chunkData != nullptr);

    // Вычисляем координаты региона
	int regionX = x >> REGION_SIZE_BIT;
	int regionZ = z >> REGION_SIZE_BIT;

    // Локальные координаты внутри региона
	int localX = x - (regionX << REGION_SIZE_BIT);
	int localZ = z - (regionZ << REGION_SIZE_BIT);

    // Создаем ключ для региона
	_tempcoords._coords[0] = regionX;
	_tempcoords._coords[1] = regionZ;
    long regionKey = _tempcoords._key;

    // Получаем или создаем регион
	char** region = regions[regionKey];
	if (region == nullptr){
		region = new char*[REGION_VOLUME];
		std::fill_n(region, REGION_VOLUME, nullptr);
		regions[regionKey] = region;
	}

    // Получаем указатель на чанк в регионе
    int chunk_index = localZ * REGION_SIZE + localX;
	char* targetChunk = region[chunk_index];

    // Если чанк не существует, создаем его
	if (targetChunk == nullptr){
		targetChunk = new char[CHUNK_VOLUME];
		region[chunk_index] = targetChunk;
		totalCompressed += CHUNK_VOLUME;
	}

    // Копируем данные
	for (uint i = 0; i < CHUNK_VOLUME; ++i)
		targetChunk[i] = chunkData[i];
}

// Формирует имя файла региона
std::string WorldFiles::getRegionFile(int x, int z) {
	return directory + std::to_string(x) + "_" + std::to_string(z) + ".bin";
}

// Получает данные чанка из кэша или файла
bool WorldFiles::getChunk(int x, int z, char* out){
	assert(out != nullptr);

    // Вычисляем координаты региона и локальные координаты
	int regionX = x >> REGION_SIZE_BIT;
	int regionZ = z >> REGION_SIZE_BIT;

	int localX = x - (regionX << REGION_SIZE_BIT);
	int localZ = z - (regionZ << REGION_SIZE_BIT);
	int chunk_index = localZ * REGION_SIZE + localX;

	assert(chunk_index >= 0 && chunk_index < REGION_VOLUME);

    // Создаем ключ региона
	_tempcoords._coords[0] = regionX;
	_tempcoords._coords[1] = regionZ;
    long region_key = _tempcoords._key;

    // Пытаемся получить из кэша
	char** region = regions[region_key];
	if (region == nullptr) return readChunk(x, z, out);

	char* chunk = region[chunk_index];
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
	int regionX = x >> REGION_SIZE_BIT;
	int regionZ = z >> REGION_SIZE_BIT;

	int localX = x - (regionX << REGION_SIZE_BIT);
	int localZ = z - (regionZ << REGION_SIZE_BIT);
	int chunk_index = localZ * REGION_SIZE + localX;

    // Открываем файл
	std::string filename = getRegionFile(regionX, regionZ);

	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()) return false;
	

    // Читаем смещение чанка в файле
	uint32_t offset;
	input.seekg(chunk_index * sizeof(uint32_t));
	input.read((char*)(&offset), sizeof(uint32_t));

	offset = bytes2Int((const unsigned char*)(&offset), 0); // Конвертируем

    // Проверяем валидность смещения
	assert (offset < 1000000);
	if (offset == 0){
		input.close();
		return false;
	}

    // Читаем размер сжатых данных
	input.seekg(offset);
	input.read((char*)(&offset), sizeof(uint32_t));
	size_t compressedSize = bytes2Int((const unsigned char*)(&offset), 0);

    // Читаем сжатые данные
	input.read(mainBufferIn, compressedSize);
	input.close();

    // Распаковываем
	decompressRLE(mainBufferIn, compressedSize, out, CHUNK_VOLUME);

	return true;
}

// Записывает все измененные регионы на диск
void WorldFiles::write(){
	for (auto& [key, region] : regions){
		if (region == nullptr) continue;

		int x, z;
		longToCoords(x, z, key);

		uint size = writeRegion(mainBufferOut, x, z, region);
		write_binary_file(getRegionFile(x,z), mainBufferOut, size);
	}
}

// Формирует бинарное представление региона для записи
uint WorldFiles::writeRegion(char* out, int x, int z, char** region){
	uint offset = REGION_VOLUME * sizeof(uint32_t);
	std::fill_n(out, offset, 0);

	char* compressed = new char[CHUNK_VOLUME * 2];  // Временный буфер для сжатия

	for (int i = 0; i < REGION_VOLUME; ++i){
		char* chunk = region[i];

        // Если чанк отсутствует в памяти, пытаемся загрузить из файла
		if (chunk == nullptr){
			chunk = new char[CHUNK_VOLUME];

			assert((((i % REGION_SIZE) + x * REGION_SIZE) >> REGION_SIZE_BIT) == x);
			assert((((i / REGION_SIZE) + z * REGION_SIZE) >> REGION_SIZE_BIT) == z);
			
            if (readChunk((i % REGION_SIZE) + x * REGION_SIZE, (i / REGION_SIZE) + z * REGION_SIZE, chunk)){
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
			uint compressedSize = compressRLE(chunk, CHUNK_VOLUME, compressed);

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
