#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

// Размеры чанка
constexpr int CHUNK_WIDTH = 16; // Ширина по X
constexpr int CHUNK_HEIGHT = 256; // Высота по Y
constexpr int CHUNK_DEPTH = 16; // Глубина по Z
constexpr int CHUNK_VOLUME = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH; // Общее количество вокселей в чанке

class voxel;
class LightMap;

// Чанк - часть воксельного мира
class Chunk {
public:
    int chunk_x, chunk_y, chunk_z; // Координаты чанка
    voxel* voxels; // Массив вокселей, содержащихся в чанке

    bool needsUpdate = true; // Нужно ли обновить меш чанка
    bool ready = false; // Готов ли чанк к рендерингу
    bool loaded = false; // Загружен ли чанк

    int surrounding = 0; // Счётчик окружающих, загруженных чанков
    int references = 1; // Счётчик ссылок

    LightMap* light_map; // Карта освещения чанка

    Chunk(int chunk_x, int chunk_y, int chunk_z); // Конструктор
    ~Chunk(); // Деструктор

    bool isEmpty(); // Проверяет, является ли чанк пустым (однородным).

    Chunk* clone() const; // Создает полную копию текущего чанка.

    void incref(); // Увеличивает счетчик ссылок на чанк.
    void decref(); // Уменьшает счётчик ссылок на чанк
};

#endif // VOXELS_CHUNK_H_
