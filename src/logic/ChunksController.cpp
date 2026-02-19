#include "ChunksController.h"

#include <limits.h>
#include <memory>

#include "../voxels/Block.h"
#include "../voxels/Chunk.h"
#include "../voxels/Chunks.h"
#include "../voxels/ChunksStorage.h"
#include "../voxels/WorldGenerator.h"
#include "../graphics/Mesh.h"
#include "../lighting/Lighting.h"
#include "../files/WorldFiles.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../constants.h"
#include "../logger/Logger.h"
#include "../definitions.h"
#include "../math/voxmaths.h"
#include "../content/Content.h"
#include "../util/timeutil.h"

constexpr int MAX_WORK_PER_FRAME = 64;
constexpr int MIN_SURROUNDING = 9;

ChunksController::ChunksController(Level* level, Chunks* chunks, Lighting* lighting, uint chunksPadding) : level(level), chunks(chunks), lighting(lighting), chunksPadding(chunksPadding), generator(new WorldGenerator(level->content)){
}

ChunksController::~ChunksController(){
    delete generator;
}

void ChunksController::update(int64_t maxDuration) {
    int64_t mcstotal = 0;
    for (uint i = 0; i < MAX_WORK_PER_FRAME; i++) {
        timeutil::Timer timer;
        if (loadVisible()) {
            int64_t mcs = timer.stop();
            avgDurationMcs = mcs * 0.2 + avgDurationMcs * 0.8;
            if (mcstotal + max(avgDurationMcs, mcs) * 2 < maxDuration * 1000) {
                mcstotal += mcs;
                continue;
            }
        }
        break;
    }
}

bool ChunksController::loadVisible(){
    const Content* content = level->content;

	const int width = chunks->width;
	const int depth = chunks->depth;
	const int areaOffsetX = chunks->areaOffsetX;
	const int areaOffsetZ = chunks->areaOffsetZ;
	int nearX = 0;
	int nearZ = 0;
	int minDistance = ((width - chunksPadding * 2) / 2) * ((width - chunksPadding * 2) / 2);
	for (uint z = chunksPadding; z < depth - chunksPadding; z++){
		for (uint x = chunksPadding; x < width - chunksPadding; x++){
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
					if (!chunk->isLoadedLights()) lighting->buildSkyLight(chunk->chunk_x, chunk->chunk_z);
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
	if (chunk != nullptr) {
		return false;
	}

    chunk = level->chunksStorage->create(nearX + areaOffsetX, nearZ + areaOffsetZ);
	chunks->putChunk(chunk);

	if (!chunk->isLoaded()) {
		generator->generate(chunk->voxels, chunk->chunk_x, chunk->chunk_z, level->world->seed);
		chunk->setUnsaved(true);
	}

    chunk->updateHeights();

	if (!chunk->isLoadedLights()) lighting->preBuildSkyLight(chunk->chunk_x, chunk->chunk_z);
	return true;
}
