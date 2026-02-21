#include "WorldGenerator.h"
#include "voxel.h"
#include "Chunk.h"
#include "Block.h"

#include <iostream>
#include <vector>
#include <time.h>
#include <stdexcept>
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#define FNL_IMPL
#include "../math/FastNoiseLite.h"

#include "../definitions.h"
#include "../logger/Logger.h"
#include "../content/Content.h"
#include "../math/voxmaths.h"
#include "../core_defs.h"

inline constexpr int SEA_LEVEL = 55;

enum class Map{
	SEND,
	TREE,
	CLIFF,
	HEIGHT
};
inline constexpr int MAPS_LEN = 4;

class Map2D {
	int x, z;
	int width, depth;
	float* heights[MAPS_LEN];
public:
	Map2D(int x, int z, int width, int depth) : x(x), z(z), width(width), depth(depth) {
		for (int i = 0; i < MAPS_LEN; ++i) {
			heights[i] = new float[width * depth];
		}
	}
	~Map2D() {
		for (int i = 0; i < MAPS_LEN; ++i) {
			delete[] heights[i];
		}
	}

	inline float get(Map map, int x, int z) {
		x -= this->x;
		z -= this->z;
		if (x < 0 || z < 0 || x >= width || z >= depth) {
			LOG_ERROR("x = {} z = {} outside of map", x, z);
			throw std::runtime_error("Out of map");
		}
		return heights[(int)map][z * width + x];
	}

	inline void set(Map map, int x, int z, float value) {
		x -= this->x;
		z -= this->z;
		if (x < 0 || z < 0 || x >= width || z >= depth) {
			LOG_ERROR("x = {} z = {} outside of map", x, z);
			throw std::runtime_error("Out of map");
		}
		heights[(int)map][z * width + x] = value;
	}
};

class PseudoRandom {
	ushort seed;
public:
	PseudoRandom() {seed = (ushort)time(0);}

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
		seed = ((ushort)(number * 3729) ^ (ushort)(number + 16786));
		rand();
	}
	void setSeed(int number1, int number2){
		seed = (((ushort)(number1 * 23729) | (ushort)(number2 % 16786)) ^ (ushort)(number2 * number1));
		rand();
	}
};



float calc_height(fnl_state *noise, int cur_x, int cur_z){
	float height = 0;

	height += fnlGetNoise2D(noise, cur_x*0.0125f*8-125567,cur_z*0.0125f*8+3546);
	height += fnlGetNoise2D(noise, cur_x*0.025f*8+4647,cur_z*0.025f*8-3436)*0.5f;
	height += fnlGetNoise2D(noise, cur_x*0.05f*8-834176,cur_z*0.05f*8+23678)*0.25f;
	height += fnlGetNoise2D(noise,
							cur_x*0.2f*8 + fnlGetNoise2D(noise, cur_x*0.1f*8-23557,cur_z*0.1f*8-6568)*50,
							cur_z*0.2f*8 + fnlGetNoise2D(noise, cur_x*0.1f*8+4363,cur_z*0.1f*8+4456)*50
							) * fnlGetNoise2D(noise, cur_x*0.01f-834176,cur_z*0.01f+23678) * 0.25;
	height += fnlGetNoise2D(noise, cur_x*0.1f*8-3465,cur_z*0.1f*8+4534)*0.125f;
	height *= fnlGetNoise2D(noise, cur_x*0.1f+1000,cur_z*0.1f+1000)*0.5f+0.5f;
	height += 1.0f;
	height *= 64.0f;
	return height;
}

WorldGenerator::WorldGenerator(const Content* content)
                : idAir(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":air")->rt.id),
                idStone(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":stone")->rt.id),
                idDirt(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":dirt")->rt.id),
				idMoss(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":moss")->rt.id),
				idSand(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":sand")->rt.id),
				idWater(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":water")->rt.id),
				idLog(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":oak_log")->rt.id),
				idLeaves(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":leaves")->rt.id),
				idGrass(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":grass")->rt.id),
				idPoppy(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":poppy")->rt.id),
				idDandelion(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":dandelion")->rt.id),
				idDaisy(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":daisy")->rt.id),
				idMarigold(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":marigold")->rt.id),
				idBedrock(content->requireBlock(DEFAULT_CONTENT_NAMESPACE":bedrock")->rt.id) {;
}

blockid_t WorldGenerator::generate_tree(fnl_state *noise, 
				  PseudoRandom* random, 
				  Map2D& heights, 
				  int cur_x, 
				  int cur_y, 
				  int cur_z, 
				  int tileSize){
	const int tileX = floordiv(cur_x, tileSize);
	const int tileZ = floordiv(cur_z, tileSize);

	random->setSeed(tileX * 4325261 + tileZ * 12160951 + tileSize * 9431111);

	int randomX = (random->rand() % (tileSize / 2)) - tileSize / 4;
	int randomZ = (random->rand() % (tileSize / 2)) - tileSize / 4;

	int centerX = tileX * tileSize + tileSize / 2 + randomX;
	int centerZ = tileZ * tileSize + tileSize / 2 + randomZ;

	bool gentree = (random->rand() % 10) < heights.get(Map::TREE, centerX, centerZ) * 13;
	if (!gentree) return 0;

	int height = (int)(heights.get(Map::HEIGHT, centerX, centerZ));
	if (height < SEA_LEVEL + 1) return 0;

	int lx = cur_x - centerX;
	int radius = random->rand() % 4 + 2;
	int ly = cur_y - height - 3 * radius;
	int lz = cur_z - centerZ;
	if (lx == 0 && lz == 0 && cur_y - height < (3 * radius + radius / 2)) return idLog;
	if (lx * lx + ly * ly / 2 + lz * lz < radius * radius) return idLeaves;
	return 0;
}

void WorldGenerator::generate(voxel* voxels, int cx, int cz, uint64_t seed){
	const int treesTile = 12;
	fnl_state noise = fnlCreateState();
	noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
	noise.seed = seed * 60617077 % 25896307;
	PseudoRandom randomtree;
	PseudoRandom randomgrass;

	int padding = 8;
	Map2D heights(cx * CHUNK_WIDTH - padding, cz * CHUNK_DEPTH - padding, CHUNK_WIDTH + padding * 2, CHUNK_DEPTH + padding * 2);

	for (int z = -padding; z < CHUNK_DEPTH + padding; ++z){
		for (int x = -padding; x < CHUNK_WIDTH + padding; ++x){
			int cur_x = x + cx * CHUNK_WIDTH;
			int cur_z = z + cz * CHUNK_DEPTH;
			float height = calc_height(&noise, cur_x, cur_z);
			float hum = fnlGetNoise2D(&noise, cur_x * 0.3 + 633, cur_z * 0.3);
			float send = fnlGetNoise2D(&noise, cur_x * 0.1 - 633, cur_z * 0.1 + 1000);
				float cliff = pow((send + abs(send)) / 2, 2);
				float width = pow(fmax(-abs(height - SEA_LEVEL) + 4, 0) / 6, 2) * cliff;
				float h1 = -abs(height - SEA_LEVEL - 0.03);
				float h2 = abs(height - SEA_LEVEL + 0.04);
				float h = (h1 + h2) * 100;
				height += (h * width);
			heights.set(Map::HEIGHT, cur_x, cur_z, height);
			heights.set(Map::TREE, cur_x, cur_z, hum);
			heights.set(Map::SEND, cur_x, cur_z, send);
			heights.set(Map::CLIFF, cur_x, cur_z, cliff);
		
		}
	}

	for (int z = 0; z < CHUNK_DEPTH; ++z){
		int cur_z = z + cz * CHUNK_DEPTH;
		for (int x = 0; x < CHUNK_WIDTH; ++x){
			int cur_x = x + cx * CHUNK_WIDTH;
			float height = heights.get(Map::HEIGHT, cur_x, cur_z);

			for (int cur_y = 0; cur_y < CHUNK_HEIGHT; ++cur_y){
				int id = cur_y < SEA_LEVEL ? idWater : idAir;
				int states = 0;
				if ((cur_y == (int)height) && (SEA_LEVEL-2 < cur_y)) {
					id = idMoss;
				} else if (cur_y < (height - 6)){
					id = idStone;
				} else if (cur_y < height){
					id = idDirt;
				} else {
					int tree = generate_tree(&noise, &randomtree, heights, cur_x, cur_y, cur_z, treesTile);
					if (tree) {
						id = tree;
						states = BLOCK_DIR_UP;
					}
				}
				float send = fmax(heights.get(Map::SEND, cur_x, cur_z), heights.get(Map::CLIFF, cur_x, cur_z));
				if (((height -  (1.1 - 0.2 * pow(height - 54, 4)) + (5 * send)) < cur_y + (height - 0.01 - (int)height)) && (cur_y < height)){
					id = idSand;
				}
				if (cur_y <= 2) id = idBedrock;

				randomgrass.setSeed(cur_x, cur_z);
                if ((id == 0) && ((height > SEA_LEVEL + 0.4) || (send > 0.1)) && ((int)(height + 1) == cur_y) && ((ushort)randomgrass.rand() > 56000)){
					id = idGrass;
				}
				if ((id == 0) && (height > SEA_LEVEL + 0.4) && ((int)(height + 1) == cur_y) && ((ushort)randomgrass.rand() > 65000)) {
					ushort flowerType = (ushort)randomgrass.rand() % 4;
					switch (flowerType) {
						case 0: id = idPoppy; break;
						case 1: id = idDandelion; break;
						case 2: id = idDaisy; break;
						case 3: id = idMarigold; break;
					}
				}
				if ((height > SEA_LEVEL + 1) && ((int)(height + 1) == cur_y) && ((ushort)randomgrass.rand() > 65533)){
					id = idLog;
					states = BLOCK_DIR_UP;
				}
				voxels[(cur_y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].id = id;
				voxels[(cur_y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].states = states;
			}
		}
	}
}
