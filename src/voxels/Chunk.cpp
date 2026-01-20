#include "Chunk.h"

#include "voxel.h"
#include "../lighting/LightMap.h"
#include "../definitions.h"

// Конструктор
Chunk::Chunk(int chunk_x, int chunk_z) : chunk_x(chunk_x), chunk_z(chunk_z) {
    voxels = new voxel[CHUNK_VOLUME];

    // Инициализируем воксели
    for(size_t i = 0; i < CHUNK_VOLUME; ++i) {
        voxels[i].id = Blocks_id::MOSS;
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

// Создает полную копию текущего чанка.
Chunk* Chunk::clone() const {
	Chunk* other = new Chunk(chunk_x, chunk_z);
	for (int i = 0; i < CHUNK_VOLUME; ++i) {
		other->voxels[i] = voxels[i];
    }
	other->light_map->set(light_map);
	return other;
}

/*
    Current chunk format:
	[voxel_ids...][voxel_states...];
*/
ubyte* Chunk::encode() const {
	ubyte* buffer = new ubyte[CHUNK_DATA_LEN];
	for (size_t i = 0; i < CHUNK_VOLUME; ++i) {
		buffer[i] = voxels[i].id;
		buffer[CHUNK_VOLUME + i] = voxels[i].states;
	}
	return buffer;
}

/*
    @return true if all is fine
*/
bool Chunk::decode(ubyte* data) {
	for (size_t i = 0; i < CHUNK_VOLUME; ++i) {
		voxel& vox = voxels[i];
		vox.id = data[i];
		vox.states = data[CHUNK_VOLUME + i];
	}
	return true;
}
