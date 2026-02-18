#ifndef FILES_WORLDFILES_H_
#define FILES_WORLDFILES_H_

#include <map>
#include <unordered_map>
#include <string>
#include <filesystem>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "../typedefs.h"

// Константы для размера регионов
namespace RegionConsts {
    constexpr uint SIZE_BIT = 5; // Размер региона 
    constexpr uint SIZE = 1 << SIZE_BIT; // Длина региона в чанках
    constexpr uint VOLUME = SIZE * SIZE; // Количество чанков в регионе
}

#define REGION_FORMAT_MAGIC ".CHROMAREG"
#define REGION_FORMAT_VERSION 1

class Player;
class Chunk;
class Content;
class ContentIndices;
class World;

class WorldRegion {
private:
	ubyte** chunksData;
	uint32_t* sizes;
	bool unsaved = true;
public:
	WorldRegion();
	~WorldRegion();

	void put(uint x, uint z, ubyte* data, uint32_t size);
	ubyte* get(uint x, uint z);
	uint getSize(uint x, uint z);

    void setUnsaved(bool unsaved) {this->unsaved = unsaved;};
	bool isUnsaved() const {return unsaved;};

	ubyte** getChunks() const {return chunksData;};
	uint32_t* getSizes() const {return sizes;};
};

// Класс для управления хранением и загрузкой данных мира в формате чанков и регионов.
class WorldFiles {
private:
    std::filesystem::path getRegionFile(int x, int z) const; // Генерирует имя файла для региона с заданными координатами
    std::filesystem::path getPlayerFile() const; // Генерирует имя файла, в котором записана информация об игроке
    std::filesystem::path getWorldFile() const; // Генерирует имя файла, в котором записана общая информауия о мире
    std::filesystem::path getRegionsFolder() const;
    std::filesystem::path getIndicesFile() const;

    WorldRegion* getRegion(int x, int z);

    ubyte* compress(ubyte* src, size_t srclen, size_t& len);
    ubyte* decompress(ubyte* src, size_t srclen, size_t dstlen);

    void writeWorldInfo(const World* world);
public:
    std::unordered_map<glm::ivec2, WorldRegion*> regions; // Хранилище регионов в оперативной памяти.
    std::filesystem::path directory; // Путь к директории с файлами мира
    ubyte* compressionBuffer; // Выходной буфер для записи регионов

    bool generatorTestMode;

    WorldFiles(std::filesystem::path directory, bool generatorTestMode); // Конструктор
    ~WorldFiles(); // Деструктор

    void put(Chunk* chunk); // Сохраняет данные чанка в кэш памяти.

    bool readWorldInfo(World* world);
    bool readPlayer(Player* player); // Читает данные об игроке с диска
    ubyte* readChunkData(int x, int y, uint32_t& length, std::filesystem::path file);
	void writeRegion(int x, int y, WorldRegion* entry, std::filesystem::path file);
	ubyte* getChunk(int x, int z); // Получает данные чанка из кэша или файла
	void writePlayer(Player* player); // Записывает данные об игроке на диск
    void write(const World* world, const Content* content);
    void writeIndices(const ContentIndices* indices);
};

#endif // FILES_WORLDFILES_H_
