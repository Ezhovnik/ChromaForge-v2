#include "Chunk.h"

#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include "voxel.h"
#include "../lighting/LightMap.h"

// Конструктор
Chunk::Chunk(int chunk_x, int chunk_y, int chunk_z) : chunk_x(chunk_x), chunk_y(chunk_y), chunk_z(chunk_z) {
    voxels = new voxel[CHUNK_VOLUME];
    for(size_t i = 0; i < CHUNK_VOLUME; ++i) {
        voxels[i].id = 1;
    }
	light_map = new LightMap();
}

// Деструктор
Chunk::~Chunk() {
    delete light_map;
    delete[] voxels;
}

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
