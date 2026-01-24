#include "ChunksStorage.h"

#include <assert.h>

#include "VoxelsVolume.h"
#include "Chunk.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../files/WorldFiles.h"
#include "../math/voxmaths.h"
#include "../lighting/Lightmap.h"
#include "../typedefs.h"

ChunksStorage::ChunksStorage(Level* level) : level(level) {
}

ChunksStorage::~ChunksStorage() {
}

void ChunksStorage::store(std::shared_ptr<Chunk> chunk) {
	chunksMap[glm::ivec2(chunk->chunk_x, chunk->chunk_z)] = chunk;
}

std::shared_ptr<Chunk> ChunksStorage::create(int x, int z) {
    auto chunk = std::shared_ptr<Chunk>(new Chunk(x, z));
    store(chunk);
    std::unique_ptr<ubyte> data(level->world->wfile->getChunk(chunk->chunk_x, chunk->chunk_z));
    if (data) {
        chunk->decode(data.get());
        chunk->setLoaded(true);
    }
    return chunk;
}

std::shared_ptr<Chunk> ChunksStorage::get(int x, int z) const {
	auto found = chunksMap.find(glm::ivec2(x, z));
	if (found == chunksMap.end()) return nullptr;
	return found->second;
}

void ChunksStorage::remove(int x, int z) {
	auto found = chunksMap.find(glm::ivec2(x, z));
	if (found != chunksMap.end()) {
		chunksMap.erase(found->first);
	}
}

void ChunksStorage::getVoxels(VoxelsVolume* volume) const {
	voxel* voxels = volume->getVoxels();
	light_t* lights = volume->getLights();
	int x = volume->getX();
	int y = volume->getY();
	int z = volume->getZ();

	int w = volume->getW();
	int h = volume->getH();
	int d = volume->getD();

	int scx = floordiv(x, CHUNK_WIDTH);
	int scz = floordiv(z, CHUNK_DEPTH);

	int ecx = floordiv(x + w, CHUNK_WIDTH);
	int ecz = floordiv(z + d, CHUNK_DEPTH);

	int cw = ecx - scx + 1;
	int ch = ecz - scz + 1;

	for (int cz = scz; cz < scz + ch; cz++) {
		for (int cx = scx; cx < scx + cw; cx++) {
			auto found = chunksMap.find(glm::ivec2(cx, cz));
			if (found == chunksMap.end()) {
				for (int ly = y; ly < y + h; ly++) {
					for (int lz = max(z, cz * CHUNK_DEPTH); lz < min(z + d, (cz + 1) * CHUNK_DEPTH); lz++) {
						for (int lx = max(x, cx * CHUNK_WIDTH); lx < min(x + w, (cx + 1) * CHUNK_WIDTH); lx++) {
							uint idx = vox_index(lx - x, ly - y, lz - z, w, d);
                            voxels[idx].id = BLOCK_VOID;
							lights[idx] = 0;
						}
					}
				}
			} else {
				const std::shared_ptr<Chunk>& chunk = found->second;
				const voxel* cvoxels = chunk->voxels;
				const light_t* clights = chunk->light_map->getLights();
				for (int ly = y; ly < y + h; ly++) {
					for (int lz = max(z, cz * CHUNK_DEPTH); lz < min(z + d, (cz + 1) * CHUNK_DEPTH); lz++) {
						for (int lx = max(x, cx * CHUNK_WIDTH); lx < min(x + w, (cx + 1) * CHUNK_WIDTH); lx++) {
							uint vidx = vox_index(lx - x, ly - y, lz - z, w, d);
                            uint cidx = vox_index(lx - cx * CHUNK_WIDTH, ly, lz - cz * CHUNK_DEPTH, CHUNK_WIDTH, CHUNK_DEPTH);
                            
                            voxels[vidx] = cvoxels[cidx];
							lights[vidx] = clights[cidx];
						}
					}
				}
			}
		}
	}
}
