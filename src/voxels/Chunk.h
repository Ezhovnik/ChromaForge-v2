#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

#include "voxel.h"

// Размеры чанка
const int CHUNK_WIDTH = 16; // Ширина по X
const int CHUNK_HEIGHT = 16; // Высота по Y
const int CHUNK_DEPTH = 16; // Глубина по Z
const int CHUNK_VOL = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH; // Общее количество вокселей в чанке

class voxel;

// Чанк - часть воксельного мира
class Chunk {
public:
    int chunk_x, chunk_y, chunk_z; // Координаты чанка
    voxel* voxels; // Массив вокселей, содержащихся в чанке
    bool needsUpdate = true; // Нужно ли обновить меш чанка

    Chunk(int chunk_x, int chunk_y, int chunk_z); // Конструктор
    ~Chunk(); // Деструктор
};

#endif // VOXELS_CHUNK_H_
