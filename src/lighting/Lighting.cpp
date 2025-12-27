#include "Lighting.h"

#include "LightSolver.h"
#include "LightMap.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"

Chunks* Lighting::chunks = nullptr;
LightSolver* Lighting::solverR = nullptr;
LightSolver* Lighting::solverG = nullptr;
LightSolver* Lighting::solverB = nullptr;
LightSolver* Lighting::solverS = nullptr;

int Lighting::initialize (Chunks* chunks) {
    Lighting::chunks = chunks;
    solverR = new LightSolver(chunks, 0);
	solverG = new LightSolver(chunks, 1);
	solverB = new LightSolver(chunks, 2);
	solverS = new LightSolver(chunks, 3);
    return 0;
}

void Lighting::finalize() {
    delete solverR, solverG, solverB, solverS;
}

void Lighting::clear() {
    for (int y = 0; y < chunks->height; ++y){
		for (int z = 0; z < chunks->depth; ++z){
			for (int x = 0; x < chunks->width; ++x){
                Chunk* chunk = chunks->getChunk(x, y, z);
                LightMap* light_map = chunk->light_map;
                for (int i = 0; i < CHUNK_VOLUME; ++i) {
                    light_map->map[i] = 0;
                }
            }
        }
    }
}

void Lighting::onWorldLoaded() {
    for (int y = 0; y < chunks->height * CHUNK_HEIGHT; ++y){
		for (int z = 0; z < chunks->depth * CHUNK_DEPTH; ++z){
			for (int x = 0; x < chunks->width * CHUNK_WIDTH; ++x){
				voxel* vox = chunks->getVoxel(x, y, z);
                Block* block = Block::blocks[vox->id];
				if (block->emission[0] || block->emission[1] || block->emission[2]){
					solverR->add(x, y, z, block->emission[0]);
					solverG->add(x, y, z, block->emission[1]);
					solverB->add(x, y, z, block->emission[2]);
				}
			}
		}
	}

	for (int z = 0; z < chunks->depth * CHUNK_DEPTH; ++z){
		for (int x = 0; x < chunks->width * CHUNK_WIDTH; ++x){
			for (int y = chunks->height * CHUNK_HEIGHT-1; y >= 0; --y){
				voxel* vox = chunks->getVoxel(x, y, z);
				if (vox->id != 0){
					break;
				}
				chunks->getChunkByVoxel(x, y, z)->light_map->setS(x % CHUNK_WIDTH, y % CHUNK_HEIGHT, z % CHUNK_DEPTH, 0xF);
			}
		}
	}

	for (int z = 0; z < chunks->depth * CHUNK_DEPTH; ++z){
		for (int x = 0; x < chunks->width * CHUNK_WIDTH; ++x){
			for (int y = chunks->height * CHUNK_HEIGHT-1; y >= 0; --y){
				voxel* vox = chunks->getVoxel(x, y, z);
				if (vox->id != 0){
					break;
				}
				if (
						chunks->getLight(x - 1, y, z, 3) == 0 ||
						chunks->getLight(x + 1, y, z, 3) == 0 ||
						chunks->getLight(x, y - 1, z, 3) == 0 ||
						chunks->getLight(x, y + 1, z, 3) == 0 ||
						chunks->getLight(x, y, z - 1, 3) == 0 ||
						chunks->getLight(x, y, z + 1, 3) == 0
						){
					solverS->add(x,y,z);
				}
				chunks->getChunkByVoxel(x, y, z)->light_map->setS(x % CHUNK_WIDTH, y % CHUNK_HEIGHT, z % CHUNK_DEPTH, 0xF);
			}
		}
	}

	solverR->solve();
	solverG->solve();
	solverB->solve();
	solverS->solve();
}

void Lighting::onBlockSet(int x, int y, int z, int id) {
    if (id == 0) {
        solverR->remove(x, y, z);
        solverG->remove(x, y, z);
        solverB->remove(x, y, z);

        solverR->solve();
        solverG->solve();
        solverB->solve();

        if (chunks->getLight(x, y + 1, z, 3) == 0xF){
            for (int i = y; i >= 0; --i){
                if (chunks->getVoxel(x, i,z )->id != 0)
                    break;
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
        solverR->remove(x,y,z);
        solverG->remove(x,y,z);
        solverB->remove(x,y,z);
        solverS->remove(x,y,z);
        for (int i = y - 1; i >= 0; --i){
            solverS->remove(x,i,z);
            if (i == 0 || chunks->getVoxel(x, i - 1, z)->id != 0){
                break;
            }
        }
        solverR->solve();
        solverG->solve();
        solverB->solve();
        solverS->solve();

        Block* block = Block::blocks[id];
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
