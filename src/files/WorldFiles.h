#ifndef FILES_WORLDFILES_H_
#define FILES_WORLDFILES_H_

#include <map>
#include <unordered_map>
#include <string>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "../typedefs.h"

// Константы для размера регионов
namespace Region_Consts {
    constexpr int REGION_SIZE_BIT = 5; // Размер региона 
    constexpr int REGION_SIZE = 1 << REGION_SIZE_BIT; // Длина региона в чанках
    constexpr int REGION_VOLUME = REGION_SIZE * REGION_SIZE; // Количество чанков в регионе
}

class Player;

struct WorldRegion {
	ubyte** chunksData;
	bool unsaved;
};

// Класс для управления хранением и загрузкой данных мира в формате чанков и регионов.
// ! Высота мира должна состовлять один чанк (любых размеров)
class WorldFiles {
public:
    static int64_t totalCompressed; // Статическая переменная для отслеживания общего объема сжатых данных
    std::unordered_map<glm::ivec2, WorldRegion> regions; // Хранилище регионов в оперативной памяти.
    std::string directory; // Путь к директории с файлами мира
    ubyte* mainBufferIn; // Входной буфер для чтения сжатых данных
    ubyte* mainBufferOut; // Выходной буфер для записи регионов

    WorldFiles(std::string directory, size_t mainBufferCapacity); // Конструктор
    ~WorldFiles(); // Деструктор

    void put(const ubyte* chunkData, int x, int z); // Сохраняет данные чанка в кэш памяти.

    bool readPlayer(Player* player); // Читает данные об игроке с диска
    bool readChunk(int x, int z, ubyte* out); // Читает данные чанка непосредственно из файла
	bool getChunk(int x, int z, ubyte* out); // Получает данные чанка из кэша или файла
	uint writeRegion(ubyte* out, int x, int z, ubyte** region); // Формирует бинарное представление региона для записи в файл
	void writePlayer(Player* player); // Записывает данные об игроке на диск
    void write(); // Записывает все измененные регионы из памяти на диск.

	std::string getRegionFile(int x, int z); // Генерирует имя файла для региона с заданными координатами
    std::string getPlayerFile(); // Генерирует имя файла, в котором записана информация об игроке
};

extern void longToCoords(int& x, int& z, long key); // Преобразует 64-битный ключ региона в координаты

#endif // FILES_WORLDFILES_H_
