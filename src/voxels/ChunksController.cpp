#include "ChunksController.h"

#include <iostream>
#include <climits>

#include <thread>

#include "Chunk.h"
#include "Chunks.h"
#include "WorldGenerator.h"
#include "../graphics/Mesh.h"
#include "../graphics/VoxelRenderer.h"
#include "../lighting/Lighting.h"
#include "../files/WorldFiles.h"
#include "ChunksLoader.h"

constexpr int MIN_SURROUNDING = 9;

ChunksController::ChunksController(Chunks* chunks, Lighting* lighting) : chunks(chunks), lighting(lighting){
	loadersCount = std::thread::hardware_concurrency() - 1;
	if (loadersCount <= 0) loadersCount = 1;
	loaders = new ChunksLoader*[loadersCount];
	for (int i = 0; i < loadersCount; ++i){
		loaders[i] = new ChunksLoader();
	}
	std::cout << "Created " << loadersCount << " loaders" << std::endl;
}

ChunksController::~ChunksController(){
	for (int i = 0; i < loadersCount; ++i)
		delete loaders[i];
	delete[] loaders;
}

int ChunksController::countFreeLoaders(){
	int count = 0;
	for (int i = 0; i < loadersCount; ++i){
		if (!loaders[i]->isBusy()) count++;
	}
	return count;
}

bool ChunksController::loadVisible(WorldFiles* worldFiles){
	const int width = chunks->width;
	const int height = chunks->height;
	const int depth = chunks->depth;

	const int areaOffsetX = chunks->areaOffsetX;
	const int areaOffsetY = chunks->areaOffsetY;
	const int areaOffsetZ = chunks->areaOffsetZ;

	int nearX = 0;
	int nearY = 0;
	int nearZ = 0;

	int minDistance = (width / 2) * (width / 2);
	for (int y = 0; y < height; ++y){
		for (int z = 2; z < depth - 2; ++z){
			for (int x = 2; x < width - 2; ++x){
				int index = (y * depth + z) * width + x;
				Chunk* chunk = chunks->chunks[index];
				if (chunk != nullptr){
					int surrounding = 0;
					for (int delta_z = -1; delta_z <= 1; delta_z++){
						for (int delta_x = -1; delta_x <= 1; delta_x++){
							Chunk* other = chunks->getChunk(chunk->chunk_x + delta_x, chunk->chunk_y, chunk->chunk_z + delta_z);
							if (other != nullptr && other->ready) surrounding++;
						}
					}
					chunk->surrounding = surrounding;
					continue;
				}
				int lx = x - width / 2;
				int ly = y - height / 2;
				int lz = z - depth / 2;
				int distance = (lx * lx + ly * ly + lz * lz);
				if (distance < minDistance){
					minDistance = distance;
					nearX = x;
					nearY = y;
					nearZ = z;
				}
			}
		}
	}

	int index = (nearY * depth + nearZ) * width + nearX;
	Chunk* chunk = chunks->chunks[index];
	if (chunk != nullptr) return false;

	ChunksLoader* freeLoader = nullptr;
	for (int i = 0; i < loadersCount; ++i){
		ChunksLoader* loader = loaders[i];
		if (loader->isBusy()) continue;
		freeLoader = loader;
		break;
	}
	if (freeLoader == nullptr) return false;

	chunk = new Chunk(nearX + areaOffsetX, nearY + areaOffsetY, nearZ + areaOffsetZ);
	if (worldFiles->getChunk(chunk->chunk_x, chunk->chunk_z, (char*)chunk->voxels)) chunk->loaded = true;

	chunks->chunks[index] = chunk;

	Chunk* closes[27];
	for (int i = 0; i < CLOSES_CNT; ++i){
		closes[i] = nullptr;
    }

	for (size_t j = 0; j < chunks->volume; ++j){
		Chunk* other = chunks->chunks[j];
		if (other == nullptr) continue;
		if (!other->ready) continue;

		int ox = other->chunk_x - chunk->chunk_x;
		int oy = other->chunk_y - chunk->chunk_y;
		int oz = other->chunk_z - chunk->chunk_z;

		if (abs(ox) > 1 || abs(oy) > 1 || abs(oz) > 1) continue;

		ox += 1;
		oy += 1;
		oz += 1;
		closes[(oy * 3 + oz) * 3 + ox] = other;
	}
	freeLoader->perform(chunk, (Chunk**)closes);
	return true;
}

bool ChunksController::_buildMeshes(VoxelRenderer* renderer, int tick) {
	const int width = chunks->width;
	const int height = chunks->height;
	const int depth = chunks->depth;

	int nearX = 0;
	int nearY = 0;
	int nearZ = 0;
	int minDistance = INT_MAX;
	for (int y = 0; y < height; y++){
		for (int z = 1; z < depth - 1; z++){
			for (int x = 1; x < width - 1; x++){
				int index = (y * depth + z) * width + x;
				Chunk* chunk = chunks->chunks[index];
				if (chunk == nullptr) continue;
				Mesh* mesh = chunks->meshes[index];
				if (mesh != nullptr && !chunk->needsUpdate) continue;
				if (!chunk->ready || chunk->surrounding < MIN_SURROUNDING) continue;
				
				int lx = x - width / 2;
				int ly = y - height / 2;
				int lz = z - depth / 2;
				int distance = (lx * lx + ly * ly + lz * lz);
				if (distance < minDistance){
					minDistance = distance;
					nearX = x;
					nearY = y;
					nearZ = z;
				}
			}
		}
	}

	int index = (nearY * depth + nearZ) * width + nearX;

	Chunk* chunk = chunks->chunks[index];
	if (chunk == nullptr) return false;
	
	Mesh* mesh = chunks->meshes[index];
	if (mesh == nullptr || chunk->needsUpdate){
		Chunk* closes[CLOSES_CNT];
		if (mesh != nullptr) delete mesh;
		if (chunk->isEmpty()){
			chunks->meshes[index] = nullptr;
			return false;
		}
		chunk->needsUpdate = false;
		for (int i = 0; i < CLOSES_CNT; ++i) {
			closes[i] = nullptr;
        }
		for (size_t j = 0; j < chunks->volume; ++j){
			Chunk* other = chunks->chunks[j];
			if (other == nullptr) continue;
			if (!other->ready) continue;

			int ox = other->chunk_x - chunk->chunk_x;
			int oy = other->chunk_y - chunk->chunk_y;
			int oz = other->chunk_z - chunk->chunk_z;

			if (abs(ox) > 1 || abs(oy) > 1 || abs(oz) > 1) continue;

			ox += 1;
			oy += 1;
			oz += 1;
			closes[(oy * 3 + oz) * 3 + ox] = other;
		}
		mesh = renderer->render(chunk, (const Chunk**)closes);
		chunks->meshes[index] = mesh;
		return true;
	}
	return false;
}
