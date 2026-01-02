#ifndef FILES_WORLDFILES_H_
#define FILES_WORLDFILES_H_

#include <map>
#include <unordered_map>
#include <string>

typedef unsigned int uint;

// Константы для размера регионов
constexpr int REGION_SIZE_BIT = 5; // Размер региона 
constexpr int REGION_SIZE = 1 << REGION_SIZE_BIT; // Длина региона в чанках
constexpr int REGION_VOLUME = REGION_SIZE * REGION_SIZE; // Количество чанков в регионе

class Player;

// Класс для управления хранением и загрузкой данных мира в формате чанков и регионов.
// ! Высота мира должна состовлять один чанк (любых размеров)
// FIXME: При движении вдоль оси Z мир зацикливается
// FIXME: Мир не совсем корректно сохранятеся (или загружается?)
class WorldFiles {
public:
    static unsigned long totalCompressed; // Статическая переменная для отслеживания общего объема сжатых данных
    std::unordered_map<long, char**> regions; // Хранилище регионов в оперативной памяти.
    std::string directory; // Путь к директории с файлами мира
    char* mainBufferIn; // Входной буфер для чтения сжатых данных
    char* mainBufferOut; // Выходной буфер для записи регионов

    WorldFiles(const char* directory, size_t mainBufferCapacity); // Конструктор
    ~WorldFiles(); // Деструктор

    void put(const char* chunkData, int x, int z); // Сохраняет данные чанка в кэш памяти.

    bool readPlayer(Player* player); // Читает данные об игроке с диска
    bool readChunk(int x, int z, char* out); // Читает данные чанка непосредственно из файла
	bool getChunk(int x, int z, char* out); // Получает данные чанка из кэша или файла
	void readRegion(char* fileContent); // Читает весь регион из файла в память.
	uint writeRegion(char* out, int x, int z, char** region); // Формирует бинарное представление региона для записи в файл
	void writePlayer(Player* player); // Записывает данные об игроке на диск
    void write(); // Записывает все измененные регионы из памяти на диск.

	std::string getRegionFile(int x, int z); // Генерирует имя файла для региона с заданными координатами
    std::string getPlayerFile(); // Генерирует имя файла, в котором записана информация об игроке
};

extern void longToCoords(int& x, int& z, long key); // Преобразует 64-битный ключ региона в координаты

#endif // FILES_WORLDFILES_H_
