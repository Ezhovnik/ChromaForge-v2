#include "ChunksController.h"

#include <iostream>
#include <climits>
#include <memory>
#include <chrono>

#include "voxel.h"
#include "Chunk.h"
#include "Chunks.h"
#include "WorldGenerator.h"
#include "Block.h"
#include "../lighting/Lighting.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../files/WorldFiles.h"
#include "ChunksStorage.h"
#include "../logger/Logger.h"
#include "../definitions.h"
#include "../math/voxmaths.h"

constexpr int MIN_SURROUNDING = 9;
constexpr int MAX_WORK_PER_FRAME = 16;

ChunksController::ChunksController(Level* level, Chunks* chunks, Lighting* lighting, uint chunksPadding) : chunks(chunks), lighting(lighting), level(level), chunksPadding(chunksPadding){
}

ChunksController::~ChunksController(){
}

void ChunksController::update(int64_t maxDuration) {
    int64_t mcstotal = 0;
    for (uint i = 0; i < MAX_WORK_PER_FRAME; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        if (loadVisible(level->world->wfile)) {
            auto elapsed = std::chrono::high_resolution_clock::now() - start;
            int64_t mcs = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
            avgDurationMcs = mcs * 0.2 + avgDurationMcs * 0.8;
            if (mcstotal + max(avgDurationMcs, mcs) * 2 < maxDuration * 1000) {
                mcstotal += mcs;
                continue;
            }
        }
        break;
    }
}

bool ChunksController::loadVisible(WorldFiles* worldFiles){
	const int width = chunks->width;
	const int depth = chunks->depth;

	const int areaOffsetX = chunks->areaOffsetX;
	const int areaOffsetZ = chunks->areaOffsetZ;

	int nearX = 0;
	int nearZ = 0;
	int minDistance = ((width - chunksPadding * 2) / 2) * ((width - chunksPadding * 2) / 2);

    for (int z = chunksPadding; z < depth - chunksPadding; ++z){
        for (int x = chunksPadding; x < width - chunksPadding; ++x){
            int index = z * width + x;
            std::shared_ptr<Chunk> chunk = chunks->chunks[index];
            if (chunk != nullptr){
                int surrounding = 0;
                for (int delta_z = -1; delta_z <= 1; delta_z++){
                    for (int delta_x = -1; delta_x <= 1; delta_x++){
                        Chunk* other = chunks->getChunk(chunk->chunk_x + delta_x, chunk->chunk_z + delta_z);
                        if (other != nullptr) surrounding++;
                    }
                }
                chunk->surrounding = surrounding;
                if (surrounding == MIN_SURROUNDING && !chunk->isLighted()) {
                    lighting->buildSkyLight(chunk->chunk_x, chunk->chunk_z);
                    lighting->onChunkLoaded(chunk->chunk_x, chunk->chunk_z);
                    chunk->setLighted(true);
                    return true;
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
	if (chunk != nullptr) return false;

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

    for (size_t i = 0; i < CHUNK_VOLUME; ++i) {
        blockid_t vox_id = chunk->voxels[i].id;
        if (Block::blocks[vox_id].get() == nullptr) {
            LOG_WARN("Corruped block detected at {} of chunk {}x {}z -> {}", i, chunk->chunk_x, chunk->chunk_z, vox_id);
            chunk->voxels[i].id = Blocks_id::BEDROCK;
        }
    }
    lighting->preBuildSkyLight(chunk->chunk_x, chunk->chunk_z);

	return true;
}
