#include <logic/ChunksController.h>

#include <limits.h>
#include <memory>

#include <voxels/Block.h>
#include <voxels/Chunk.h>
#include <voxels/Chunks.h>
#include <voxels/ChunksStorage.h>
#include <world/generator/WorldGenerator.h>
#include <graphics/core/Mesh.h>
#include <lighting/Lighting.h>
#include <files/WorldFiles.h>
#include <world/Level.h>
#include <world/World.h>
#include <constants.h>
#include <debug/Logger.h>
#include <core_content_defs.h>
#include <math/voxmaths.h>
#include <content/Content.h>
#include <util/timeutil.h>
#include <content/Content.h>

inline constexpr int MAX_WORK_PER_FRAME = 128;
inline constexpr int MIN_SURROUNDING = 9;

ChunksController::ChunksController(
	Level* level, 
	uint chunksPadding
) : level(level), 
	chunks(level->chunks.get()), 
	lighting(level->lighting.get()), 
	chunksPadding(chunksPadding), 
	generator(std::make_unique<WorldGenerator>(
        level->content->generators.require(level->getWorld()->getGenerator()),
        level->content,
		level->getWorld()->getSeed()
    )) {}

ChunksController::~ChunksController() = default;

void ChunksController::update(
    int64_t maxDuration, int loadDistance, int centerX, int centerY
) {
    generator->update(centerX, centerY, loadDistance);
    int64_t mcstotal = 0;
    for (uint i = 0; i < MAX_WORK_PER_FRAME; ++i) {
        timeutil::Timer timer;
        if (loadVisible()) {
            int64_t mcs = timer.stop();
            if (mcstotal + mcs < maxDuration * 1000) {
                mcstotal += mcs;
                continue;
            }
        }
        break;
    }
}

bool ChunksController::loadVisible() {
	int sizeX = chunks->getWidth();
    int sizeY = chunks->getDepth();

	int nearX = 0;
	int nearZ = 0;
	int minDistance = ((sizeX - chunksPadding * 2) / 2) * ((sizeY - chunksPadding * 2) / 2);
	for (uint z = chunksPadding; z < sizeY - chunksPadding; ++z){
		for (uint x = chunksPadding; x < sizeX - chunksPadding; ++x){
			int index = z * sizeX + x;
			auto& chunk = chunks->getChunks()[index];
			if (chunk != nullptr) {
				if (chunk->flags.loaded && !chunk->flags.lighted) {
					if (buildLights(chunk)) {
						return true;
					}
				}
				continue;
			}
			int lx = x - sizeX / 2;
			int lz = z - sizeY / 2;
			int distance = (lx * lx + lz * lz);
			if (distance < minDistance){
				minDistance = distance;
				nearX = x;
				nearZ = z;
			}
		}
	}

	const auto& chunk = chunks->getChunks()[nearZ * sizeX + nearX];
	if (chunk != nullptr) {
		return false;
	}

	int offsetX = chunks->getOffsetX();
    int offsetZ = chunks->getOffsetZ();
    createChunk(nearX + offsetX, nearZ + offsetZ);
	return true;
}

bool ChunksController::buildLights(const std::shared_ptr<Chunk>& chunk) {
    int surrounding = 0;
    for (int dz = -1; dz <= 1; ++dz) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (chunks->getChunk(chunk->chunk_x + dx, chunk->chunk_z + dz)) surrounding++;
        }
    }

    if (surrounding == MIN_SURROUNDING) {
        bool lightsCache = chunk->flags.loadedLights;
        if (!lightsCache) lighting->buildSkyLight(chunk->chunk_x, chunk->chunk_z);
        lighting->onChunkLoaded(chunk->chunk_x, chunk->chunk_z, !lightsCache);
        chunk->flags.lighted = true;
        return true;
    }
    return false;
}

void ChunksController::createChunk(int x, int z) {
    auto chunk = level->chunksStorage->create(x, z);
	chunks->putChunk(chunk);
	auto& chunkFlags = chunk->flags;

	if (!chunkFlags.loaded) {
		generator->generate(chunk->voxels, x, z);
		chunkFlags.unsaved = true;
	}
	chunk->updateHeights();

	if (!chunkFlags.loadedLights) Lighting::preBuildSkyLight(chunk.get(), level->content->getIndices());

    chunkFlags.loaded = true;
	chunkFlags.ready = true;
}
