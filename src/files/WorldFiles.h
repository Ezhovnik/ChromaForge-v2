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

#define REGION_FORMAT_VERSION 1
#define REGION_FORMAT_MAGIC ".CHROMAREG"

#define WORLD_FORMAT_VERSION 1
#define WORLD_FORMAT_MAGIC ".CHROMAWLD"

// Константы для размера регионов
namespace RegionConsts {
    constexpr int REGION_SIZE_BIT = 5; // Размер региона 
    constexpr int REGION_SIZE = 1 << REGION_SIZE_BIT; // Длина региона в чанках
    constexpr int REGION_VOLUME = REGION_SIZE * REGION_SIZE; // Количество чанков в регионе
}

class Player;
class Chunk;

struct WorldRegion {
	ubyte** chunksData;
    uint32_t* compressedSizes;
	bool unsaved;
};

struct WorldInfo {
    std::string name;
    std::filesystem::path directory;
    uint64_t seed;
};

// Класс для управления хранением и загрузкой данных мира в формате чанков и регионов.
class WorldFiles {
private:
    void writeWorldInfo(const WorldInfo& info);
	std::filesystem::path getRegionFile(int x, int y) const;
	std::filesystem::path getPlayerFile() const;
    std::filesystem::path getWorldFile() const;
public:
    std::unordered_map<glm::ivec2, WorldRegion> regions; // Хранилище регионов в оперативной памяти.
    std::filesystem::path directory; // Путь к директории с файлами мира
    ubyte* compressionBuffer;

    bool generatorTestMode;

    WorldFiles(std::filesystem::path directory, bool generatorTestMode); // Конструктор
    ~WorldFiles(); // Деструктор

    void put(Chunk* chunk); // Сохраняет данные чанка в кэш памяти.

    bool readWorldInfo(WorldInfo& info);
    bool readPlayer(Player* player); // Читает данные об игроке с диска
    ubyte* readChunkData(int x, int z, uint32_t& length);
	ubyte* getChunk(int x, int z); // Получает данные чанка из кэша или файла
	void writeRegion(int x, int z, WorldRegion& entry); // Формирует бинарное представление региона для записи в файл
	void writePlayer(Player* player); // Записывает данные об игроке на диск
    void write(const WorldInfo info);
};

#endif // FILES_WORLDFILES_H_
