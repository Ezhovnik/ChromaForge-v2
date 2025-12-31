#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

// Размеры чанка
constexpr int CHUNK_WIDTH = 16; // Ширина по X
constexpr int CHUNK_HEIGHT = 64; // Высота по Y
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
    LightMap* light_map;

    Chunk(int chunk_x, int chunk_y, int chunk_z); // Конструктор
    ~Chunk(); // Деструктор

    bool isEmpty();
};

#endif // VOXELS_CHUNK_H_
