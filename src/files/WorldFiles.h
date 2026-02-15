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
    constexpr int SIZE_BIT = 5; // Размер региона 
    constexpr int SIZE = 1 << SIZE_BIT; // Длина региона в чанках
    constexpr int VOLUME = SIZE * SIZE; // Количество чанков в регионе
}

#define REGION_FORMAT_MAGIC ".CHROMAREG"
#define REGION_FORMAT_VERSION 1

#define WORLD_FORMAT_MAGIC ".CHROMAWLD"
#define WORLD_FORMAT_VERSION 1

class Player;
class Chunk;
class Content;
class ContentIndices;

struct WorldRegion {
	ubyte** chunksData;
    uint32_t* compressedSizes;
	bool unsaved;
};

struct WorldInfo {
    std::string name;
    std::filesystem::path directory;
    uint64_t seed;
    float daytime;
	float daytimeSpeed;
};

// Класс для управления хранением и загрузкой данных мира в формате чанков и регионов.
class WorldFiles {
private:
    std::filesystem::path getRegionFile(int x, int z) const; // Генерирует имя файла для региона с заданными координатами
    std::filesystem::path getPlayerFile() const; // Генерирует имя файла, в котором записана информация об игроке
    std::filesystem::path getWorldFile() const; // Генерирует имя файла, в котором записана общая информауия о мире
    std::filesystem::path getRegionsFolder() const;
    std::filesystem::path getBlockIndicesFile() const;

    void writeWorldInfo(const WorldInfo& info);
public:
    std::unordered_map<glm::ivec2, WorldRegion> regions; // Хранилище регионов в оперативной памяти.
    std::filesystem::path directory; // Путь к директории с файлами мира
    ubyte* compressionBuffer; // Выходной буфер для записи регионов

    bool generatorTestMode;

    WorldFiles(std::filesystem::path directory, bool generatorTestMode); // Конструктор
    ~WorldFiles(); // Деструктор

    void put(Chunk* chunk); // Сохраняет данные чанка в кэш памяти.

    bool readWorldInfo(WorldInfo& info);
    bool readPlayer(Player* player); // Читает данные об игроке с диска
    ubyte* readChunkData(int x, int z, uint32_t& length); // Читает данные чанка непосредственно из файла
	ubyte* getChunk(int x, int z); // Получает данные чанка из кэша или файла
	void writeRegion(int x, int y, WorldRegion& entry); // Формирует бинарное представление региона для записи в файл
	void writePlayer(Player* player); // Записывает данные об игроке на диск
    void write(const WorldInfo info, const Content* content);
    void writeIndices(const ContentIndices* indices);
};

extern void longToCoords(int& x, int& z, long key); // Преобразует 64-битный ключ региона в координаты

#endif // FILES_WORLDFILES_H_
