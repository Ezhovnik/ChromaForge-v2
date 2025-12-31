#include "WorldGenerator.h"
#include "voxel.h"
#include "Chunk.h"

#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

// Генерирует воксельные данные для чанка
void WorldGenerator::generate(voxel* voxels, int chunk_x, int chunk_y, int chunk_z){
    // Проходим по всем вокселям в чанке
	for (int z = 0; z < CHUNK_DEPTH; ++z){
        int real_z = z + chunk_z * CHUNK_DEPTH;
		for (int x = 0; x < CHUNK_WIDTH; ++x){
			int real_x = x + chunk_x * CHUNK_WIDTH;

            // Генерация высоты рельефа с использованием шума Перлина
			float height = glm::perlin(glm::vec3(real_x * 0.0125f,real_z * 0.0125f, 0.0f));
			height += glm::perlin(glm::vec3(real_x * 0.025f,real_z * 0.025f, 0.0f)) * 0.5f;

			height *= 0.1f;
			height += 0.05f;

            // Генерация высоты рельефа с использованием шума Перлина
			for (int y = 0; y < CHUNK_HEIGHT; ++y){
				int real_y = y + chunk_y * CHUNK_HEIGHT;
				float noise = height;
				int voxel_id = noise / std::fmax(0.01f, real_y * 0.1f + 0.1f) > 0.1f;
        
				if (real_y <= 2) voxel_id = 2;
				if (voxel_id == 0 && real_y == 14 && height <= 0.01f) voxel_id = 1;

				voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].id = voxel_id;
			}
		}
	}
}
