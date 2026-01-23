#include "WorldGenerator.h"

#include <iostream>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#define FNL_IMPL
#include "../math/FastNoiseLite.h"

#include "voxel.h"
#include "Chunk.h"
#include "../definitions.h"
#include "../logger/Logger.h"
#include "../math/voxmaths.h"
#include "../typedefs.h"

inline constexpr int SEA_LEVEL = 55;

class Map2D {
	int x, z;
	int width, depth;
	float* heights;
public:
	Map2D(int x, int z, int width, int depth) : x(x), z(z), width(width), depth(depth) {
		heights = new float[width * depth];
	}
	~Map2D() {
		delete[] heights;
	}

	inline float get(int x, int z) {
		x -= this->x;
		z -= this->z;
		if (x < 0 || z < 0 || x >= width || z >= depth) {
            LOG_ERROR("Out of heightmap");
			throw std::runtime_error("out of heightmap");
		}
		return heights[z * width + x];
	}

	inline void set(int x, int z, float value) {
		x -= this->x;
		z -= this->z;
		if (x < 0 || z < 0 || x >= width || z >= depth) {
            LOG_ERROR("Out of heightmap");
			throw std::runtime_error("out of heightmap");
		}
		heights[z * width + x] = value;
	}
};

class PseudoRandom {
	ushort seed;
public:
	PseudoRandom(){
		seed = (ushort)time(0);
	}

	int rand(){
		seed = (seed + 0x7ed5 + (seed << 6));
		seed = (seed ^ 0xc23c ^ (seed >> 9));
		seed = (seed + 0x1656 + (seed << 3));
		seed = ((seed + 0xa264) ^ (seed << 4));
		seed = (seed + 0xfd70 - (seed << 3));
		seed = (seed ^ 0xba49 ^ (seed >> 8));

		return (int)seed;
	}

	void setSeed(int number){
		seed = ((ushort)(number + 23729) ^ (ushort)(number + 16786));
		rand();
	}
    void setSeed(int number1, int number2){
		seed = (((ushort)(number1 * 23729) | (ushort)(number2 % 16786)) ^ (ushort)(number2 * number1));
		rand();
	}
};

float calc_height(fnl_state *noise, int real_x, int real_z){
	float height = 0;

	height += fnlGetNoise3D(noise, real_x*0.0125f*8-125567,real_z*0.0125f*8+3546, 0.0f);
	height += fnlGetNoise3D(noise, real_x*0.025f*8+4647,real_z*0.025f*8-3436, 0.0f)*0.5f;
	height += fnlGetNoise3D(noise, real_x*0.05f*8-834176,real_z*0.05f*8+23678, 0.0f)*0.25f;
	height += fnlGetNoise3D(noise,
			real_x*0.2f*8 + fnlGetNoise3D(noise, real_x*0.1f*8-23557,real_z*0.1f*8-6568, 0.0f)*50,
			real_z*0.2f*8 + fnlGetNoise3D(noise, real_x*0.1f*8+4363,real_z*0.1f*8+4456, 0.0f)*50,
			0.0f) * fnlGetNoise3D(noise, real_x*0.01f-834176,real_z*0.01f+23678, 0.0f);
	height += fnlGetNoise3D(noise, real_x*0.1f*8-3465,real_z*0.1f*8+4534, 0.0f)*0.125f;
	//height += fnlGetNoise3D(noise, real_x*0.4f+4565,real_z*0.4f*18+46456, 0.0f)*0.0625f * 0.3f;
	height *= fnlGetNoise3D(noise, real_x*0.1f+1000,real_z*0.1f+1000, 0.0f)*0.5f+0.5f;
	height += 1.0f;
	height *= 64.0f;
	return height;
}

blockid_t generate_tree(fnl_state *noise, PseudoRandom* random, Map2D& heights, Map2D& humidity, int real_x, int real_y, int real_z, int tileSize){
	const int tileX = floordiv(real_x, tileSize);
	const int tileZ = floordiv(real_z, tileSize);

	random->setSeed(tileX * 4325261 + tileZ * 12160951 + tileSize * 9431111);

    int randomX = (random->rand() % (tileSize/2)) - tileSize/4;
	int randomZ = (random->rand() % (tileSize/2)) - tileSize/4;

	int centerX = tileX * tileSize + tileSize/2 + randomX;
	int centerZ = tileZ * tileSize + tileSize/2 + randomZ;

	bool gentree = (random->rand() % 10) < humidity.get(centerX, centerZ) * 13;
	if (!gentree) return Blocks_id::AIR;

	int height = (int)(heights.get(centerX, centerZ));
	if (height < SEA_LEVEL + 1) return Blocks_id::AIR;
	int lx = real_x - centerX;
	int radius = random->rand() % 4 + 2;
	int ly = real_y - height - 3 * radius;
	int lz = real_z - centerZ;
	if (lx == 0 && lz == 0 && real_y - height < (3 * radius + radius / 2)) return Blocks_id::LOG;
	if (lx * lx + ly * ly / 2 + lz * lz < radius * radius) return Blocks_id::LEAVES;
	return Blocks_id::AIR;
}

void WorldGenerator::generate(voxel* voxels, int cx, int cz, int seed) {
    const int treesTile = 12;
	fnl_state noise = fnlCreateState();
	noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
	noise.seed = seed * 60617077 % 25896307;
	PseudoRandom randomtree;
    PseudoRandom randomgrass;

	int padding = 8;
	Map2D heights(cx * CHUNK_WIDTH - padding, cz * CHUNK_DEPTH - padding, CHUNK_WIDTH + padding * 2, CHUNK_DEPTH + padding * 2);
	Map2D humidity(cx * CHUNK_WIDTH - padding, cz * CHUNK_DEPTH - padding, CHUNK_WIDTH + padding * 2, CHUNK_DEPTH + padding * 2);

	for (int z = -padding; z < CHUNK_DEPTH + padding; z++){
        int real_z = z + cz * CHUNK_DEPTH;
		for (int x = -padding; x < CHUNK_WIDTH + padding; x++){
			int real_x = x + cx * CHUNK_WIDTH;
			float height = calc_height(&noise, real_x, real_z);
            float hum = fnlGetNoise3D(&noise, real_x * 0.3 + 633, 0.0, real_z * 0.3);
			if (height >= SEA_LEVEL) {
				height = ((height - SEA_LEVEL) * 0.1) - 0.0;
				height = powf(height, (1.0+hum - fmax(0.0, height) * 0.2));
				height = height * 10 + SEA_LEVEL;
			} else {
				height *= 1.0f + (height-SEA_LEVEL) * 0.05f * hum;
			}
			heights.set(real_x, real_z, height);
			humidity.set(real_x, real_z, hum);
		}
	}

	for (int z = 0; z < CHUNK_DEPTH; z++){
        int real_z = z + cz * CHUNK_DEPTH;
        for (int x = 0; x < CHUNK_WIDTH; x++){
            int real_x = x + cx * CHUNK_WIDTH;
            float height = heights.get(real_x, real_z);
            for (int y = 0; y < CHUNK_HEIGHT; y++){
                int real_y = y;
                blockid_t id = real_y < SEA_LEVEL ? Blocks_id::WATER : Blocks_id::AIR;
                uint8_t states = 0;
                
                if ((real_y == (int)height) && (SEA_LEVEL - 2 < real_y)) {
                    id = Blocks_id::MOSS;
                } else if (real_y < (height - 6)){
                    id = Blocks_id::STONE;
                } else if (real_y < height){
                    id = Blocks_id::DIRT;
                } else {
                    int tree = generate_tree(&noise, &randomtree, heights, humidity, real_x, real_y, real_z, treesTile);
                    if (tree != Blocks_id::AIR) {
                        id = tree;
                        states = BLOCK_DIR_Y;
                    }
                }
                
                if (((height - (1.5 - 0.2 * pow(height - 54, 4))) < real_y) && (real_y < height) && humidity.get(real_x, real_z) < 0.1) id = Blocks_id::SAND;
                
                if (real_y <= 2) id = Blocks_id::BEDROCK;

                randomgrass.setSeed(real_x, real_z);
                if ((id == Blocks_id::AIR) && (real_y > SEA_LEVEL + 0.5) && ((int)(height + 1) == real_y)) {
                    ushort grass_value = (ushort)randomgrass.rand();
                    
                    if (grass_value > 62000 || (real_y > 70 && grass_value > 61500)) {
                        id = Blocks_id::GRASS;
                    } else if (grass_value > 61000) {
                        id = Blocks_id::DAISY;
                    } else if (grass_value > 60000) {
                        id = Blocks_id::POPPY;
                    } else if (grass_value > 59000) {
                        id = Blocks_id::DANDELION;
                    }
                }
                
                if ((height > SEA_LEVEL + 1) && ((int)(height + 1) == real_y) && ((ushort)randomgrass.rand() > 65533)){
                    id = Blocks_id::LOG;
                    states = BLOCK_DIR_Y;
                }
                voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].id = id;
                voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].states = states;
            }
        }
    }
}
