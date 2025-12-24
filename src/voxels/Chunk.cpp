#include "Chunk.h"

// Конструктор
Chunk::Chunk() {
    voxels = new voxel[CHUNK_VOL]; // Выделяет память под массив вокселей

    // Генерация чанка
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            for (int x = 0; x < CHUNK_WIDTH; ++x) {
                voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].id = y <= 5;
            }
        }
    }
}

// Деструктор
Chunk::~Chunk() {
    delete[] voxels;
}
