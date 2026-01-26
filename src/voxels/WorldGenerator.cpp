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
#include "../declarations.h"
#include "../typedefs.h"

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
		seed = ((ushort)number + 23729 xor (ushort)number + 16786);
		rand();
	}
    void setSeed(int number1,int number2){
		seed = (((unsigned short)(number1 * 23729) | (unsigned short)(number2 % 16786)) ^ (unsigned short)(number2 * number1));
		rand();
	}
};

float calc_height(fnl_state *noise, int real_x, int real_z){
	float height = fnlGetNoise3D(noise, real_x*0.0125f*8,real_z*0.0125f*8, 0.0f);
	height += fnlGetNoise3D(noise, real_x*0.025f*8,real_z*0.025f*8, 0.0f)*0.5f;
	height += fnlGetNoise3D(noise, real_x*0.05f*8,real_z*0.05f*8, 0.0f)*0.25f;
	height += fnlGetNoise3D(noise,
			real_x*0.2f*8 + fnlGetNoise3D(noise, real_x*0.1f*8,real_z*0.1f*8, 0.0f)*50,
			real_z*0.2f*8 + fnlGetNoise3D(noise, real_x*0.1f*8+4363,real_z*0.1f*8, 0.0f)*50,
			0.0f)*0.1f;
	height += fnlGetNoise3D(noise, real_x*0.1f*8,real_z*0.1f*8, 0.0f)*0.125f;
	height += fnlGetNoise3D(noise, real_x*0.4f*8,real_z*0.4f*8, 0.0f)*0.0625f;
	// height += fnlGetNoise3D(noise, real_x*8,real_z*8, 0.0f)*0.03f*(fnlGetNoise3D(noise, -real_x*0.0125f*8-1000,real_z*0.0125f*8+2000, 0.0f)/2+0.5f);
	height *= fnlGetNoise3D(noise, real_x*0.0125f*8+1000,real_z*0.0125f*8+1000, 0.0f)/2+0.5f;
	height += 1.0f;
	height *= 64.0f;
	return height;
}

float calc_height_faster(fnl_state *noise, int real_x, int real_z){
	float height = fnlGetNoise3D(noise, real_x*0.0125f*8,real_z*0.0125f*8, 0.0f);
	height += fnlGetNoise3D(noise, real_x*0.025f*8,real_z*0.025f*8, 0.0f)*0.5f;
	height += fnlGetNoise3D(noise, real_x*0.05f*8,real_z*0.05f*8, 0.0f)*0.25f;
	height += fnlGetNoise3D(noise,
			real_x*0.2f*8 + fnlGetNoise3D(noise, real_x*0.1f*8,real_z*0.1f*8, 0.0f)*50,
			real_z*0.2f*8 + fnlGetNoise3D(noise, real_x*0.1f*8+4363,real_z*0.1f*8, 0.0f)*50,
			0.0f)*0.1f;
	height += fnlGetNoise3D(noise, real_x*0.1f*8,real_z*0.1f*8, 0.0f)*0.125f;
	height *= fnlGetNoise3D(noise, real_x*0.0125f*8+1000,real_z*0.0125f*8+1000, 0.0f)/2+0.5f;
	height += 1.0f;
	height *= 64.0f;
	return height;
}

int generate_tree(fnl_state *noise, PseudoRandom* random, const std::vector<float>& heights, int real_x, int real_y, int real_z, int tileSize){
	const int tileX = floor((double)real_x/(double)tileSize);
	const int tileY = floor((double)real_z/(double)tileSize);
	random->setSeed(tileX*4325261+tileY*12160951+tileSize*9431111);

	bool gentree = fnlGetNoise3D(noise, tileX*3.0f+633, 0.0, tileY*3.0f) > -0.1f && (random->rand() % 10) < 7;
	if (!gentree) return BlockID::AIR;

	const int randomX = (random->rand() % (tileSize/2)) - tileSize/4;
	const int randomZ = (random->rand() % (tileSize/2)) - tileSize/4;
	int centerX = tileX * tileSize + tileSize/2 + randomX;
	int centerY = tileY * tileSize + tileSize/2 + randomZ;
	int height = (int)calc_height_faster(noise, centerX, centerY);
	if ((height < 57) /*|| (fnlGetNoise3D(noise, real_x*0.025f,real_z*0.025f, 0.0f)*0.5f > 0.5)*/) return BlockID::AIR;
	int lx = real_x - centerX;
	int radius = random->rand() % 4 + 2;
	int ly = real_y - height - 3 * radius;
	int lz = real_z - centerY;
	if (lx == 0 && lz == 0 && real_y - height < (3 * radius + radius / 2)) return BlockID::LOG;
	if (lx*lx+ly*ly/2+lz*lz < radius*radius) return BlockID::LEAVES;
	return BlockID::AIR;
}

void WorldGenerator::generate(voxel* voxels, int cx, int cz, int seed){
	fnl_state noise = fnlCreateState();
	noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
	noise.seed = seed * 60617077 % 25896307;
	PseudoRandom randomtree;
    PseudoRandom randomgrass;

	std::vector<float> heights(CHUNK_VOLUME);

	for (int z = 0; z < CHUNK_DEPTH; z++){
		for (int x = 0; x < CHUNK_WIDTH; x++){
			int real_x = x + cx * CHUNK_WIDTH;
			int real_z = z + cz * CHUNK_DEPTH;
			float height = calc_height(&noise, real_x, real_z);
			heights[z * CHUNK_WIDTH + x] = height;
		}
	}

	for (int z = 0; z < CHUNK_DEPTH; z++){
        int real_z = z + cz * CHUNK_DEPTH;
        for (int x = 0; x < CHUNK_WIDTH; x++){
            int real_x = x + cx * CHUNK_WIDTH;
            float height = heights[z * CHUNK_WIDTH + x];
            for (int y = 0; y < CHUNK_HEIGHT; y++){
                int real_y = y;
                int id = real_y < 55 ? BlockID::WATER : BlockID::AIR;
                int states = 0;
                
                if ((real_y == (int)height) && (54 < real_y)) {
                    id = BlockID::MOSS;
                } else if (real_y < (height - 6)){
                    id = BlockID::STONE;
                } else if (real_y < height){
                    id = BlockID::DIRT;
                } else {
                    int tree = generate_tree(&noise, &randomtree, heights, real_x, real_y, real_z, 23);
                    if (tree) {
                        id = tree;
                        states = 0x32;
                    }
                }
                
                if (((height - (1.5 - 0.2 * pow(height - 54, 4))) < real_y) && (real_y < height)) id = BlockID::SAND;
                
                if (real_y <= 2) id = BlockID::BEDROCK;

                if ((id == BlockID::AIR) && (real_y > 55.5) && ((int)(height + 1) == real_y)) {
                    randomgrass.setSeed(real_x, real_z);
                    unsigned short grass_value = (unsigned short)randomgrass.rand();
                    
                    if (grass_value > 62000 || (real_y > 70 && grass_value > 61500)) {
                        id = BlockID::GRASS;
                    } else if (grass_value > 61000) {
                        id = BlockID::DAISY;
                    } else if (grass_value > 60000) {
                        id = BlockID::POPPY;
                    } else if (grass_value > 59000) {
                        id = BlockID::DANDELION;
                    }
                }
                
                if ((height > 56) && ((int)(height + 1) == real_y) && ((unsigned short)randomgrass.rand() > 65533)){
                    id = BlockID::LOG;
                    states = 0x32;
                }
                voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].id = id;
                voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].states = states;
            }
        }
    }
}
