#include "Chunk.h"

#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include "voxel.h"

// Конструктор
Chunk::Chunk(int chunk_x, int chunk_y, int chunk_z) : chunk_x(chunk_x), chunk_y(chunk_y), chunk_z(chunk_z) {
    voxels = new voxel[CHUNK_VOLUME]; // Выделяет память под массив вокселей

    // Генерация чанка
    for (int z = 0; z < CHUNK_DEPTH; ++z) {
        int real_z = z + this->chunk_z * CHUNK_DEPTH;
        for (int x = 0; x < CHUNK_WIDTH; ++x) {
            int real_x = x + this->chunk_x * CHUNK_WIDTH;
            float height = glm::perlin(glm::vec3(real_x * 0.025f, real_z * 0.025f, 0.0f));
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                int real_y = y + this->chunk_y * CHUNK_HEIGHT;
                int vox_id = real_y <= height * 10 + 5;
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
