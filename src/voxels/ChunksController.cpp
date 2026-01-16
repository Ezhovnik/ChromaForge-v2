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
#include "../logger/Logger.h"

constexpr int MIN_SURROUNDING = 9;

ChunksController::ChunksController(World* world, Chunks* chunks, Lighting* lighting) : chunks(chunks), lighting(lighting){
	loadersCount = std::thread::hardware_concurrency() * 2 - 1;
	if (loadersCount <= 0) loadersCount = 1;

	loaders = new ChunksLoader*[loadersCount];

	for (int i = 0; i < loadersCount; ++i){
		loaders[i] = new ChunksLoader(world);
	}

    LOG_INFO("Created {} chunks loaders", loadersCount);
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
	const int depth = chunks->depth;

	const int areaOffsetX = chunks->areaOffsetX;
	const int areaOffsetZ = chunks->areaOffsetZ;

	int nearX = 0;
	int nearZ = 0;
	int minDistance = (width / 2) * (width / 2);

    for (int z = 2; z < depth - 2; ++z){
        for (int x = 2; x < width - 2; ++x){
            int index = z * width + x;
            Chunk* chunk = chunks->chunks[index];
            if (chunk != nullptr){
                int surrounding = 0;
                for (int delta_z = -1; delta_z <= 1; delta_z++){
                    for (int delta_x = -1; delta_x <= 1; delta_x++){
                        Chunk* other = chunks->getChunk(chunk->chunk_x + delta_x, chunk->chunk_z + delta_z);
                        if (other != nullptr && other->isReady()) surrounding++;
                    }
                }
                chunk->surrounding = surrounding;
                continue;
            }
            int lx = x - width / 2;
            int lz = z - depth / 2;
            int distance = (lx * lx + lz * lz);
            if (distance < minDistance){
                minDistance = distance;
                nearX = x;
                nearZ = z;
            }
        }
    }

	int index = nearZ * width + nearX;
	Chunk* chunk = chunks->chunks[index];
	if (chunk != nullptr) return false;

	ChunksLoader* freeLoader = getFreeLoader();
    if (freeLoader == nullptr) return false;

	chunk = new Chunk(nearX + areaOffsetX, nearZ + areaOffsetZ);
	if (worldFiles->getChunk(chunk->chunk_x, chunk->chunk_z, (ubyte*)chunk->voxels)) chunk->setLoaded(true);

	chunks->putChunk(chunk);

	Chunk* closes[9];
	for (int i = 0; i < 9; ++i) {
        closes[i] = nullptr;
    }

	for (size_t j = 0; j < chunks->volume; ++j){
		Chunk* other = chunks->chunks[j];
		if (other == nullptr || !other->isReady()) continue;

		int ox = other->chunk_x - chunk->chunk_x;
		int oz = other->chunk_z - chunk->chunk_z;

		if (abs(ox) > 1 || abs(oz) > 1) continue;

		ox++;
		oz++;
		closes[oz * 3 + ox] = other;
	}
	freeLoader->load(chunk, (Chunk**)closes);
	return true;
}

ChunksLoader* ChunksController::getFreeLoader() {
	ChunksLoader* freeLoader = nullptr;

	for (int i = 0; i < loadersCount; ++i){
		ChunksLoader* loader = loaders[i];
        if (!loader->isBusy()) {
            freeLoader = loader;
            break;
        }
	}

	return freeLoader;
}

void ChunksController::calculateLights() {
	ChunksLoader* freeLoader = getFreeLoader();
	if (freeLoader == nullptr) return;

	const int width = chunks->width;
	const int depth = chunks->depth;

	int nearX = 0;
	int nearZ = 0;
	int minDistance = INT_MAX;

	for (int z = 1; z < depth - 1; z++){
		for (int x = 1; x < width - 1; x++){
			int index = z * width + x;
			Chunk* chunk = chunks->chunks[index];

			if (chunk == nullptr || chunk->isLighted() || chunk->surrounding < MIN_SURROUNDING) continue;
			
			int lx = x - width / 2;
			int lz = z - depth / 2;
			int distance = (lx * lx + lz * lz);
			if (distance < minDistance){
				minDistance = distance;
				nearX = x;
				nearZ = z;
			}
		}
	}
	int index = nearZ * width + nearX;
	Chunk* chunk = chunks->chunks[index];
	if (chunk == nullptr) return;

	Chunk* closes[9];
	for (int i = 0; i < 9; ++i) {
        closes[i] = nullptr;
    }

	for (size_t j = 0; j < chunks->volume; ++j){
		Chunk* other = chunks->chunks[j];
		if (other == nullptr) continue;

		int ox = other->chunk_x - chunk->chunk_x;
		int oz = other->chunk_z - chunk->chunk_z;

		if (abs(ox) > 1|| abs(oz) > 1) continue;

		ox++;
		oz++;
		closes[oz * 3 + ox] = other;
	}
	freeLoader->lights(chunk, (Chunk**)closes);
}

bool ChunksController::_buildMeshes() {
	const int width = chunks->width;
	const int depth = chunks->depth;

    for (int z = 1; z < depth - 1; z++){
        for (int x = 1; x < width - 1; x++){
            int index = z * width + x;

            Chunk* chunk = chunks->chunks[index];
            if (chunk == nullptr) continue;

            if (chunk->renderData.vertices > (void*)1){
                Mesh* mesh = new Mesh(chunk->renderData.vertices, chunk->renderData.size / VoxelRenderer_Conts::CHUNK_VERTEX_SIZE, VoxelRenderer_Conts::CHUNK_ATTRS);

                if (chunks->meshes[index]) delete chunks->meshes[index];
				chunks->meshes[index] = mesh;

				delete[] chunk->renderData.vertices;
				chunk->renderData.vertices = nullptr;
            }
        }
    }

    ChunksLoader* freeLoader = getFreeLoader();
	if (freeLoader == nullptr) return false;

    int nearX = 0;
	int nearZ = 0;
	int minDistance = INT_MAX;
	for (int z = 1; z < depth - 1; z++){
		for (int x = 1; x < width - 1; x++){
			int index = z * width + x;

			Chunk* chunk = chunks->chunks[index];
			if (chunk == nullptr || !chunk->isReady() || !chunk->isLighted() || chunk->surrounding < MIN_SURROUNDING) continue;

			Mesh* mesh = chunks->meshes[index];
			if (mesh != nullptr && !chunk->isModified()) continue;
			
			int lx = x - width / 2;
			int lz = z - depth / 2;
			int distance = (lx * lx + lz * lz);
			if (distance < minDistance){
				minDistance = distance;
				nearX = x;
				nearZ = z;
			}
		}
	}
	int index = nearZ * width + nearX;
	Chunk* chunk = chunks->chunks[index];
	if (chunk == nullptr) return false;
	
	Mesh* mesh = chunks->meshes[index];
	if (mesh == nullptr || chunk->isModified()){
		if (chunk->renderData.vertices != nullptr) return false;
        if (chunk->isEmpty()){
			chunks->meshes[index] = nullptr;
			return false;
		}
		
		Chunk* closes[9];
		for (int i = 0; i < 9; ++i) {
            closes[i] = nullptr;
        }

		for (size_t j = 0; j < chunks->volume; j++){
			Chunk* other = chunks->chunks[j];
			if (other == nullptr) continue;

			int ox = other->chunk_x - chunk->chunk_x;
			int oz = other->chunk_z - chunk->chunk_z;

			if (abs(ox) > 1 || abs(oz) > 1) continue;

			ox++;
			oz++;
			
            if ((!other->isReady() || !other->isLighted()) && other != chunk) return false;
			closes[oz * 3 + ox] = other;
		}
		chunk->setModified(false);
		chunk->renderData.vertices = (float*)1;
		freeLoader->render(chunk, (Chunk**)closes);

		return true;
	}
	return false;
}
