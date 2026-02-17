#include "Lighting.h"

#include <memory>
#include <string>

#include "LightSolver.h"
#include "LightMap.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"
#include "../definitions.h"
#include "../typedefs.h"
#include "../constants.h"
#include "../content/Content.h"

Lighting::Lighting(const Content* content, Chunks* chunks) : chunks(chunks), content(content) {
    const ContentIndices* contentIds = content->indices;
	solverR = new LightSolver(contentIds, chunks, 0);
	solverG = new LightSolver(contentIds, chunks, 1);
	solverB = new LightSolver(contentIds, chunks, 2);
	solverS = new LightSolver(contentIds, chunks, 3);

	airID = content->require(DEFAULT_BLOCK_NAMESPACE"air")->rt.id;
}

Lighting::~Lighting(){
	delete solverR;
	delete solverG;
	delete solverB;
	delete solverS;
}

void Lighting::clear() {
    for (uint index = 0; index < chunks->volume; ++index) {
        std::shared_ptr<Chunk> chunk = chunks->chunks[index];
        if (chunk == nullptr) continue;
        
        LightMap* light_map = chunk->light_map;
        for (int i = 0; i < CHUNK_VOLUME; ++i) {
            light_map->map[i] = 0;
        }
    }
}

void Lighting::preBuildSkyLight(int cx, int cz){
    const Block* const* blockDefs = content->indices->getBlockDefs();

	Chunk* chunk = chunks->getChunk(cx, cz);
	int highestPoint = 0;
	for (int z = 0; z < CHUNK_DEPTH; z++){
		for (int x = 0; x < CHUNK_WIDTH; x++){
			for (int y = CHUNK_HEIGHT - 1;;y--){
				if (y < 0) break;
				voxel* vox = &(chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]);
				const Block* block = blockDefs[vox->id];
				if (!block->skyLightPassing) {
					if (highestPoint < y) highestPoint = y;
					break;
				}
				chunk->light_map->setS(x, y, z, 15);
			}
		}
	}
	if (highestPoint < CHUNK_HEIGHT - 1) highestPoint++;
	chunk->light_map->highestPoint = highestPoint;
}

void Lighting::buildSkyLight(int cx, int cz) {
    const Block* const* blockDefs = content->indices->getBlockDefs();

	Chunk* chunk = chunks->getChunk(cx, cz);
	for (int z = 0; z < CHUNK_DEPTH; ++z){
		for (int x = 0; x < CHUNK_WIDTH; ++x){
			for (int y = chunk->light_map->highestPoint; y >= 0; y--){
				int gx = x + cx * CHUNK_WIDTH;
				int gz = z + cz * CHUNK_DEPTH;
				while (y > 0 && !blockDefs[chunk->voxels[vox_index(x, y, z)].id]->lightPassing) {
					y--;
				}
				if (chunk->light_map->getS(x, y, z) != 15) {
					solverS->add(gx, y + 1, gz);
					for (; y >= 0; y--){
						solverS->add(gx+1,y,gz);
						solverS->add(gx-1,y,gz);
						solverS->add(gx,y,gz+1);
						solverS->add(gx,y,gz-1);
					}
				}
			}
		}
	}
	solverS->solve();
}

void Lighting::onChunkLoaded(int chunk_x, int chunk_z) {
    const Block* const* blockDefs = content->indices->getBlockDefs();
    const Chunk* chunk = chunks->getChunk(chunk_x, chunk_z);

	for (uint y = 0; y < CHUNK_HEIGHT; ++y){
		for (uint z = 0; z < CHUNK_DEPTH; ++z){
			for (uint x = 0; x < CHUNK_WIDTH; ++x){
				voxel vox = chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
				const Block* block = blockDefs[vox.id];
				int gx = x + chunk_x * CHUNK_WIDTH;
				int gz = z + chunk_z * CHUNK_DEPTH;
				if (block->emission[0] || block->emission[1] || block->emission[2]){
					solverR->add(gx,y,gz,block->emission[0]);
					solverG->add(gx,y,gz,block->emission[1]);
					solverB->add(gx,y,gz,block->emission[2]);
				}
			}
		}
	}

	for (int y = -1; y <= CHUNK_HEIGHT; ++y){
		for (int z = -1; z <= CHUNK_DEPTH; ++z){
			for (int x = -1; x <= CHUNK_WIDTH; ++x){
				if (!(x == -1 || x == CHUNK_WIDTH || z == -1 || z == CHUNK_DEPTH)) continue;
				int gx = x + chunk_x * CHUNK_WIDTH;
				int gz = z + chunk_z * CHUNK_DEPTH;
				if (chunks->getLight(x, y, z)){
					solverR->add(gx, y, gz);
					solverG->add(gx, y, gz);
					solverB->add(gx, y, gz);
					solverS->add(gx, y, gz);
				}
			}
		}
	}

	solverR->solve();
	solverG->solve();
	solverB->solve();
	solverS->solve();
}

void Lighting::onBlockSet(int x, int y, int z, int const id) {
    Block* block = content->indices->getBlockDef(id);
    if (id == airID) {
        solverR->remove(x, y, z);
        solverG->remove(x, y, z);
        solverB->remove(x, y, z);

        solverR->solve();
        solverG->solve();
        solverB->solve();

        if (chunks->getLight(x, y + 1, z, 3) == 0xF){
            for (int i = y; i >= 0; --i){
                voxel* vox = chunks->getVoxel(x, i, z);
                if ((vox == nullptr || vox->id != airID) && block->skyLightPassing) break;
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
        solverR->remove(x, y, z);
        solverG->remove(x, y, z);
        solverB->remove(x, y, z);
    
        if (!block->skyLightPassing) {
            solverS->remove(x, y, z);
            for (int i = y - 1; i >= 0; --i){
                solverS->remove(x, i, z);
                if (i == 0 || chunks->getVoxel(x, i - 1, z)->id != airID) break;
            }
            solverS->solve();
        }
        solverR->solve();
        solverG->solve();
        solverB->solve();

		if (block->emission[0] || block->emission[1] || block->emission[2]){
			solverR->add(x, y, z, block->emission[0]);
			solverG->add(x, y, z, block->emission[1]);
			solverB->add(x, y, z, block->emission[2]);

			solverR->solve();
			solverG->solve();
			solverB->solve();
        }
    }
}
