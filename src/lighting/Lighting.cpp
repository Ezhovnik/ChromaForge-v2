#include "Lighting.h"

#include "LightSolver.h"
#include "LightMap.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"
#include "../declarations.h"
#include "../typedefs.h"

Lighting::Lighting(Chunks* chunks) : chunks(chunks){
	solverR = new LightSolver(chunks, 0);
	solverG = new LightSolver(chunks, 1);
	solverB = new LightSolver(chunks, 2);
	solverS = new LightSolver(chunks, 3);
}

Lighting::~Lighting(){
	delete solverR;
	delete solverG;
	delete solverB;
	delete solverS;
}

void Lighting::clear() {
    for (uint index = 0; index < chunks->volume; ++index) {
        Chunk* chunk = chunks->chunks[index];
        if (chunk == nullptr) continue;
        
        LightMap* light_map = chunk->light_map;
        for (int i = 0; i < CHUNK_VOLUME; ++i) {
            light_map->map[i] = 0;
        }
    }
}

void Lighting::preBuildSkyLight(int cx, int cz){
	Chunk* chunk = chunks->getChunk(cx, cz);
	int highestPoint = 0;
	for (int z = 0; z < CHUNK_DEPTH; z++){
		for (int x = 0; x < CHUNK_WIDTH; x++){
			for (int y = CHUNK_HEIGHT - 1;;y--){
				if (y < 0) break;
				voxel* vox = &(chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]);
				Block* block = Block::blocks[vox->id].get();
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

void Lighting::buildSkyLight(int cx, int cz){
	Chunk* chunk = chunks->getChunk(cx, cz);
	for (int z = 0; z < CHUNK_DEPTH; ++z){
		for (int x = 0; x < CHUNK_WIDTH; ++x){
			for (int y = chunk->light_map->highestPoint; y >= 0; y--){
				int gx = x + cx * CHUNK_WIDTH;
				int gz = z + cz * CHUNK_DEPTH;
				while (y > 0 && !Block::blocks[chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].id].get()->lightPassing) {
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
    const Chunk* chunk = chunks->getChunk(chunk_x, chunk_z);

	for (uint y = 0; y < CHUNK_HEIGHT; ++y){
		for (uint z = 0; z < CHUNK_DEPTH; ++z){
			for (uint x = 0; x < CHUNK_WIDTH; ++x){
				voxel vox = chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
				Block* block = Block::blocks[vox.id].get();
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

void Lighting::onBlockSet(int x, int y, int z, int id) {
    Block* block = Block::blocks[id].get();
    if (id == Blocks_id::AIR) {
        solverR->remove(x, y, z);
        solverG->remove(x, y, z);
        solverB->remove(x, y, z);

        solverR->solve();
        solverG->solve();
        solverB->solve();

        if (chunks->getLight(x, y + 1, z, 3) == 0xF){
            for (int i = y; i >= 0; --i){
                voxel* vox = chunks->getVoxel(x, i, z);
                if ((vox == nullptr || vox->id != Blocks_id::AIR) && Block::blocks[id].get()->skyLightPassing) break;
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
                if (i == 0 || chunks->getVoxel(x, i - 1, z)->id != Blocks_id::AIR) break;
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
