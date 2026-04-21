#include <lighting/Lighting.h>

#include <memory>
#include <string>

#include <lighting/LightSolver.h>
#include <lighting/LightMap.h>
#include <voxels/Chunks.h>
#include <voxels/Chunk.h>
#include <voxels/voxel.h>
#include <voxels/Block.h>
#include <core_content_defs.h>
#include <typedefs.h>
#include <constants.h>
#include <content/Content.h>

Lighting::Lighting(const Content* content, Chunks* chunks) : chunks(chunks), content(content) {
    const ContentIndices* contentIds = content->getIndices();
	solverR = std::make_unique<LightSolver>(contentIds, chunks, 0);
	solverG = std::make_unique<LightSolver>(contentIds, chunks, 1);
	solverB = std::make_unique<LightSolver>(contentIds, chunks, 2);
	solverS = std::make_unique<LightSolver>(contentIds, chunks, 3);
}

Lighting::~Lighting() = default;

void Lighting::clear() {
    for (size_t index = 0; index < chunks->volume; ++index) {
        auto chunk = chunks->chunks[index];
        if (chunk == nullptr) continue;

        LightMap& light_map = chunk->light_map;
        for (int i = 0; i < CHUNK_VOLUME; ++i) {
            light_map.map[i] = 0;
        }
    }
}

void Lighting::preBuildSkyLight(Chunk* chunk, const ContentIndices* indices){
    const auto* blockDefs = indices->blocks.getDefs();
	int highestPoint = 0;
	for (int z = 0; z < CHUNK_DEPTH; ++z){
		for (int x = 0; x < CHUNK_WIDTH; ++x){
			for (int y = CHUNK_HEIGHT - 1; y >= 0; --y){
				int index = (y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x;
				voxel& vox = chunk->voxels[index];
				const Block* block = blockDefs[vox.id];
				if (!block->skyLightPassing) {
					if (highestPoint < y) highestPoint = y;
					break;
				}
				chunk->light_map.setS(x, y, z, 15);
			}
		}
	}
	if (highestPoint < CHUNK_HEIGHT - 1) highestPoint++;
	chunk->light_map.highestPoint = highestPoint;
}

void Lighting::buildSkyLight(int cx, int cz) {
    const auto blockDefs = content->getIndices()->blocks.getDefs();

	Chunk* chunk = chunks->getChunk(cx, cz);
	for (int z = 0; z < CHUNK_DEPTH; ++z) {
		int gz = z + cz * CHUNK_DEPTH;
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			int gx = x + cx * CHUNK_WIDTH;
			for (int y = chunk->light_map.highestPoint; y >= 0; --y){
				while (y > 0 && !blockDefs[chunk->voxels[vox_index(x, y, z)].id]->lightPassing) {
					--y;
				}
				if (chunk->light_map.getS(x, y, z) != 15) {
					solverS->add(gx, y + 1, gz);
					for (; y >= 0; y--){
						solverS->add(gx + 1, y, gz);
						solverS->add(gx - 1, y, gz);
						solverS->add(gx, y, gz + 1);
						solverS->add(gx, y, gz - 1);
					}
				}
			}
		}
	}
	solverS->solve();
}

void Lighting::onChunkLoaded(int chunk_x, int chunk_z, bool expand) {
	LightSolver* solverR = this->solverR.get();
    LightSolver* solverG = this->solverG.get();
    LightSolver* solverB = this->solverB.get();
    LightSolver* solverS = this->solverS.get();

    auto blockDefs = content->getIndices()->blocks.getDefs();
    auto chunk = chunks->getChunk(chunk_x, chunk_z);

	for (uint y = 0; y < CHUNK_HEIGHT; ++y){
		for (uint z = 0; z < CHUNK_DEPTH; ++z){
			for (uint x = 0; x < CHUNK_WIDTH; ++x){
				const voxel& vox = chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
				const Block* block = blockDefs[vox.id];
				int gx = x + chunk_x * CHUNK_WIDTH;
				int gz = z + chunk_z * CHUNK_DEPTH;
				if (block->rt.emissive){
					solverR->add(gx, y, gz, block->emission[0]);
					solverG->add(gx, y, gz, block->emission[1]);
					solverB->add(gx, y, gz, block->emission[2]);
				}
			}
		}
	}

	if (expand) {
		for (int x = 0; x < CHUNK_WIDTH; x += CHUNK_WIDTH - 1) {
			int gx = x + chunk_x * CHUNK_WIDTH;
			for (int y = 0; y < CHUNK_HEIGHT; ++y) {
				for (int z = 0; z < CHUNK_DEPTH; ++z) {
					int gz = z + chunk_z * CHUNK_DEPTH;

					int rgbs = chunk->light_map.get(x, y, z);
					if (rgbs){
						solverR->add(gx, y, gz, LightMap::extract(rgbs, 0));
						solverG->add(gx, y, gz, LightMap::extract(rgbs, 1));
						solverB->add(gx, y, gz, LightMap::extract(rgbs, 2));
						solverS->add(gx, y, gz, LightMap::extract(rgbs, 3));
					}
				}
			}
		}

		for (int z = 0; z < CHUNK_DEPTH; z += CHUNK_DEPTH - 1) {
			int gz = z + chunk_z * CHUNK_DEPTH;
			for (int y = 0; y < CHUNK_HEIGHT; ++y) {
				for (int x = 0; x < CHUNK_WIDTH; ++x) {
					int gx = x + chunk_x * CHUNK_WIDTH;
					int rgbs = chunk->light_map.get(x, y, z);
					if (rgbs) {
						solverR->add(gx, y, gz, LightMap::extract(rgbs, 0));
						solverG->add(gx, y, gz, LightMap::extract(rgbs, 1));
						solverB->add(gx, y, gz, LightMap::extract(rgbs, 2));
						solverS->add(gx, y, gz, LightMap::extract(rgbs, 3));
					}
				}
			}
		}
	}

	solverR->solve();
	solverG->solve();
	solverB->solve();
	solverS->solve();
}

void Lighting::onBlockSet(int x, int y, int z, blockid_t id) {
    const auto& block = content->getIndices()->blocks.require(id);

	solverR->remove(x, y, z);
    solverG->remove(x, y, z);
    solverB->remove(x, y, z);

    if (id == BLOCK_AIR) {
        solverR->solve();
        solverG->solve();
        solverB->solve();

        if (chunks->getLight(x, y + 1, z, 3) == 0xF){
            for (int i = y; i >= 0; --i){
                voxel* vox = chunks->getVoxel(x, i, z);
                if ((vox == nullptr || vox->id != BLOCK_AIR) && block.skyLightPassing) break;
                solverS->add(x, i, z, 0xF);
            }
        }

        solverR->add(x, y + 1, z); solverG->add(x, y + 1, z); solverB->add(x, y + 1, z); solverS->add(x, y + 1, z);
        solverR->add(x, y - 1, z); solverG->add(x, y - 1, z); solverB->add(x, y - 1, z); solverS->add(x, y - 1, z);
        solverR->add(x + 1, y, z); solverG->add(x + 1, y, z); solverB->add(x + 1, y, z); solverS->add(x + 1, y, z);
        solverR->add(x - 1, y, z); solverG->add(x - 1, y, z); solverB->add(x - 1, y, z); solverS->add(x - 1, y, z);
        solverR->add(x, y, z + 1); solverG->add(x, y, z + 1); solverB->add(x, y, z + 1); solverS->add(x, y, z + 1);
        solverR->add(x, y, z - 1); solverG->add(x, y, z - 1); solverB->add(x, y, z - 1); solverS->add(x, y, z - 1);

        solverR->solve();
        solverG->solve();
        solverB->solve();
        solverS->solve();
    } else {
        if (!block.skyLightPassing) {
            solverS->remove(x, y, z);
            for (int i = y - 1; i >= 0; --i){
                solverS->remove(x, i, z);
                if (i == 0 || chunks->getVoxel(x, i - 1, z)->id != BLOCK_AIR) break;
            }
            solverS->solve();
        }
        solverR->solve();
        solverG->solve();
        solverB->solve();

		if (block.emission[0] || block.emission[1] || block.emission[2]) {
			solverR->add(x, y, z, block.emission[0]);
			solverG->add(x, y, z, block.emission[1]);
			solverB->add(x, y, z, block.emission[2]);

			solverR->solve();
			solverG->solve();
			solverB->solve();
        }
    }
}
