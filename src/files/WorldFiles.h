#ifndef FILES_WORLDFILES_H_
#define FILES_WORLDFILES_H_

#include <map>
#include <unordered_map>
#include <string>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "../typedefs.h"

#define REGION_FORMAT_VERSION 1

// Константы для размера регионов
namespace Region_Consts {
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

// Класс для управления хранением и загрузкой данных мира в формате чанков и регионов.
class WorldFiles {
public:
    std::unordered_map<glm::ivec2, WorldRegion> regions; // Хранилище регионов в оперативной памяти.
    std::string directory; // Путь к директории с файлами мира
    ubyte* compressionBuffer;

    bool generatorTestMode;

    WorldFiles(std::string directory, size_t mainBufferCapacity, bool generatorTestMode); // Конструктор
    ~WorldFiles(); // Деструктор

    void put(Chunk* chunk); // Сохраняет данные чанка в кэш памяти.

    bool readPlayer(Player* player); // Читает данные об игроке с диска
    ubyte* readChunkData(int x, int z, uint32_t& length);
	ubyte* getChunk(int x, int z); // Получает данные чанка из кэша или файла
	void writeRegion(int x, int z, WorldRegion& entry); // Формирует бинарное представление региона для записи в файл
	void writePlayer(Player* player); // Записывает данные об игроке на диск
    void write(); // Записывает все измененные регионы из памяти на диск.

	std::string getRegionFile(int x, int z); // Генерирует имя файла для региона с заданными координатами
    std::string getPlayerFile(); // Генерирует имя файла, в котором записана информация об игроке
};

extern void longToCoords(int& x, int& z, long key); // Преобразует 64-битный ключ региона в координаты

#endif // FILES_WORLDFILES_H_
