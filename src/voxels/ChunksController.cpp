#include "ChunksController.h"

#include <limits.h>
#include <memory>

#include "Block.h"
#include "Chunk.h"
#include "Chunks.h"
#include "ChunksStorage.h"
#include "WorldGenerator.h"
#include "../graphics/Mesh.h"
#include "../lighting/Lighting.h"
#include "../files/WorldFiles.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../constants.h"
#include "../logger/Logger.h"
#include "../declarations.h"

#define MIN_SURROUNDING 9

ChunksController::ChunksController(Level* level, Chunks* chunks, Lighting* lighting) : level(level), chunks(chunks), lighting(lighting){
}

ChunksController::~ChunksController(){
}

bool ChunksController::loadVisible(WorldFiles* worldFiles){
	const int width = chunks->width;
	const int depth = chunks->depth;
	const int areaOffsetX = chunks->areaOffsetX;
	const int areaOffsetZ = chunks->areaOffsetZ;
	int nearX = 0;
	int nearZ = 0;
	int minDistance = (width / 2) * (width / 2);
	for (int z = 2; z < depth - 2; z++){
		for (int x = 2; x < width - 2; x++){
			int index = z * width + x;
			std::shared_ptr<Chunk> chunk = chunks->chunks[index];
			if (chunk != nullptr){
				int surrounding = 0;
				for (int dz = -1; dz <= 1; dz++){
					for (int dx = -1; dx <= 1; dx++){
						Chunk* other = chunks->getChunk(chunk->chunk_x + dx, chunk->chunk_z + dz);
						if (other != nullptr) surrounding++;
					}
				}
				chunk->surrounding = surrounding;
				if (surrounding == MIN_SURROUNDING && !chunk->isLighted()) {
					lighting->buildSkyLight(chunk->chunk_x, chunk->chunk_z);
					lighting->onChunkLoaded(chunk->chunk_x, chunk->chunk_z);
					chunk->setLighted(true);
					return false;
				}
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
	std::shared_ptr<Chunk> chunk = chunks->chunks[index];
	if (chunk != nullptr) {
		return false;
	}

	chunk = std::shared_ptr<Chunk>(new Chunk(nearX + areaOffsetX, nearZ + areaOffsetZ));
	level->chunksStorage->store(chunk);
	ubyte* data = worldFiles->getChunk(chunk->chunk_x, chunk->chunk_z);
	if (data) {
		chunk->decode(data);
		chunk->setLoaded(true);

        delete[] data;
    }

	chunks->putChunk(chunk);

	if (!chunk->isLoaded()) {
		WorldGenerator::generate(chunk->voxels, chunk->chunk_x, chunk->chunk_z, level->world->seed);
		chunk->setUnsaved(true);
	}

	for (size_t i = 0; i < CHUNK_VOLUME; i++) {
        blockid_t id = chunk->voxels[i].id;
		if (Block::blocks[id] == nullptr) {
            LOG_WARN("Corruped block id = {} detected at {} of chunk {}x {}z", id, i, chunk->chunk_x, chunk->chunk_z);
			chunk->voxels[i].id = BlockID::BEDROCK;
		}
	}
	lighting->preBuildSkyLight(chunk->chunk_x, chunk->chunk_z);
	return true;
}
