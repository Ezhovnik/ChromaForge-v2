#include "Chunk.h"

#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include "voxel.h"
#include "../lighting/LightMap.h"

// Конструктор
Chunk::Chunk(int chunk_x, int chunk_z) : chunk_x(chunk_x), chunk_z(chunk_z) {
    bottom = 0;
	top = CHUNK_HEIGHT;
    voxels = new voxel[CHUNK_VOLUME];

    // Инициализируем воксели
    for(size_t i = 0; i < CHUNK_VOLUME; ++i) {
        voxels[i].id = 0;
        voxels[i].states = 0;
    }

	light_map = new LightMap();
    renderData.vertices = nullptr;
}

// Деструктор
Chunk::~Chunk() {
    delete light_map;
    delete[] voxels;
    delete[] renderData.vertices;
}

// Проверяет, является ли чанк пустым (однородным).
bool Chunk::isEmpty() {
    int id = -1;
	for (int i = 0; i < CHUNK_VOLUME; ++i){
		if (voxels[i].id != id){
			if (id != -1) return false;
			else id = voxels[i].id;
		}
	}
	return true;
}

void Chunk::updateHeights() {
	for (int i = 0; i < CHUNK_VOLUME; i++) {
		if (voxels[i].id != 0) {
			bottom = i / (CHUNK_DEPTH * CHUNK_WIDTH);
			break;
		}
	}

	for (int i = CHUNK_VOLUME - 1; i > -1; i--) {
		if (voxels[i].id != 0) {
			top = i / (CHUNK_DEPTH * CHUNK_WIDTH) + 1;
			break;
		}
	}
}

// Создает полную копию текущего чанка.
Chunk* Chunk::clone() const {
	Chunk* other = new Chunk(chunk_x, chunk_z);
	for (int i = 0; i < CHUNK_VOLUME; ++i) {
		other->voxels[i] = voxels[i];
    }
	other->light_map->set(light_map);
	return other;
}

// Формат: [voxel_ids...][voxel_states...];
ubyte* Chunk::encode() const {
	ubyte* buffer = new ubyte[CHUNK_DATA_LEN];
	for (size_t i = 0; i < CHUNK_VOLUME; i++) {
		buffer[i] = voxels[i].id;
		buffer[CHUNK_VOLUME + i] = voxels[i].states;
	}
	return buffer;
}

bool Chunk::decode(ubyte* data) {
	for (size_t i = 0; i < CHUNK_VOLUME; i++) {
		voxel& vox = voxels[i];
		vox.id = data[i];
		vox.states = data[CHUNK_VOLUME + i];
	}
	return true;
}
