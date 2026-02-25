#include "Chunk.h"

#include <math.h>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include "voxel.h"
#include "../lighting/LightMap.h"
#include "../content/ContentLUT.h"

// Конструктор
Chunk::Chunk(int chunk_x, int chunk_z) : chunk_x(chunk_x), chunk_z(chunk_z) {
    bottom = 0;
	top = CHUNK_HEIGHT;
    voxels = new voxel[CHUNK_VOLUME];

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
	for (size_t i = 0; i < CHUNK_VOLUME; ++i){
		if (voxels[i].id != id){
			if (id != -1) return false;
			else id = voxels[i].id;
		}
	}
	return true;
}

void Chunk::updateHeights() {
	for (size_t i = 0; i < CHUNK_VOLUME; i++) {
		if (voxels[i].id != 0) {
			bottom = i / (CHUNK_DEPTH * CHUNK_WIDTH);
			break;
		}
	}

	for (int i = CHUNK_VOLUME - 1; i >= 0; i--) {
		if (voxels[i].id != 0) {
			top = i / (CHUNK_DEPTH * CHUNK_WIDTH) + 1;
			break;
		}
	}
}

// Создает полную копию текущего чанка.
Chunk* Chunk::clone() const {
	Chunk* other = new Chunk(chunk_x, chunk_z);
	for (size_t i = 0; i < CHUNK_VOLUME; ++i) {
		other->voxels[i] = voxels[i];
    }
	other->light_map->set(light_map);
	return other;
}

// Формат: [voxel_ids...][voxel_states...];
ubyte* Chunk::encode() const {
	ubyte* buffer = new ubyte[CHUNK_DATA_LEN];
	for (size_t i = 0; i < CHUNK_VOLUME; ++i) {
		buffer[i] = voxels[i].id >> 8;
        buffer[CHUNK_VOLUME + i] = voxels[i].id & 0xFF;
		buffer[CHUNK_VOLUME * 2 + i] = voxels[i].states >> 8;
        buffer[CHUNK_VOLUME * 3 + i] = voxels[i].states & 0xFF;
	}
	return buffer;
}

bool Chunk::decode(ubyte* data) {
	for (size_t i = 0; i < CHUNK_VOLUME; ++i) {
		voxel& vox = voxels[i];

		ubyte bid1 = data[i];
        ubyte bid2 = data[CHUNK_VOLUME + i];
        
        ubyte bst1 = data[CHUNK_VOLUME * 2 + i];
        ubyte bst2 = data[CHUNK_VOLUME * 3 + i];

		vox.id = (blockid_t(bid1) << 8) | (blockid_t(bid2));
        vox.states = (blockstate_t(bst1) << 8) | (blockstate_t(bst2));
	}
	return true;
}

void Chunk::fromOld(ubyte* data) {
    for (size_t i = 0; i < CHUNK_VOLUME; ++i) {
        data[i + CHUNK_VOLUME * 3] = data[i + CHUNK_VOLUME];
        data[i + CHUNK_VOLUME] = data[i];
        data[i + CHUNK_VOLUME * 2] = 0;
        data[i] = 0;
    }
}

void Chunk::convert(ubyte* data, const ContentLUT* lut) {
    for (size_t i = 0; i < CHUNK_VOLUME; ++i) {
        blockid_t id = ((blockid_t(data[i]) << 8) | blockid_t(data[CHUNK_VOLUME + i]));
        blockid_t replacement = lut->getBlockId(id);
        data[i] = replacement >> 8;
        data[CHUNK_VOLUME + i] = replacement & 0xFF;
    }
}
