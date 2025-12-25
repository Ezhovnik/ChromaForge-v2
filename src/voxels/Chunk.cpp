#include "Chunk.h"

#include <math.h>

// Конструктор
Chunk::Chunk(int chunk_x, int chunk_y, int chunk_z) : chunk_x(chunk_x), chunk_y(chunk_y), chunk_z(chunk_z) {
    voxels = new voxel[CHUNK_VOL]; // Выделяет память под массив вокселей

    // Генерация чанка
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            for (int x = 0; x < CHUNK_WIDTH; ++x) {
                int real_x = x + this->chunk_x * CHUNK_WIDTH;
                int real_y = y + this->chunk_y * CHUNK_HEIGHT;
                int real_z = z + this->chunk_z * CHUNK_DEPTH;

                int vox_id = real_y <= (sin(real_x * 0.1f) * 0.5f + 0.5f) * 10;
                if (real_y <= 2) {
                    vox_id = 2;
                }

                voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].id = vox_id;
            }
        }
    }
}

// Деструктор
Chunk::~Chunk() {
    delete[] voxels;
}
