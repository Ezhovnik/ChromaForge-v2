#include "Lighting.h"

#include "LightSolver.h"
#include "LightMap.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"

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
    for (unsigned int index = 0; index < chunks->volume; ++index) {
        Chunk* chunk = chunks->chunks[index];
        if (chunk == nullptr) continue;
        
        LightMap* light_map = chunk->light_map;
        for (int i = 0; i < CHUNK_VOLUME; ++i) {
            light_map->map[i] = 0;
        }
    }
}

void Lighting::onChunkLoaded(int chunk_x, int chunk_y, int chunk_z, bool sky) {
    Chunk* chunk = chunks->getChunk(chunk_x, chunk_y, chunk_z);
	Chunk* chunkUpper = chunks->getChunk(chunk_x, chunk_y + 1, chunk_z);
	Chunk* chunkLower = chunks->getChunk(chunk_x, chunk_y - 1, chunk_z);
	if (chunkLower && sky){
		for (int z = 0; z < CHUNK_DEPTH; z++){
			for (int x = 0; x < CHUNK_WIDTH; x++){
				int gx = x + chunk_x * CHUNK_WIDTH;
				int gy = chunk_y * CHUNK_HEIGHT;
				int gz = z + chunk_z * CHUNK_DEPTH;

				int light = chunk->light_map->getS(x,0,z);
				int new_chunk_y = chunk_y - 1;
				if (light < 15){
					Chunk* current = chunkLower;
					if (chunkLower->light_map->getS(x, 15, z) == 0) continue;
					for (int y = 15;;y--){
						if (y < 0){
							new_chunk_y--;
							y += CHUNK_HEIGHT;
						}
						if (new_chunk_y != current->chunk_y) current = chunks->getChunk(chunk_x, new_chunk_y, chunk_z);
						if (!current) break;
						voxel* vox = &(current->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]);
						Block* block = Block::blocks[vox->id];
						if (!block->lightPassing) break;
						current->needsUpdate = true;
						solverS->remove(gx, y + new_chunk_y * CHUNK_HEIGHT, gz);
						current->light_map->setS(x, y, z, 0);
					}
				}
			}
		}
	}
	if (chunkUpper && sky){
		for (int z = 0; z < CHUNK_DEPTH; z++){
			for (int x = 0; x < CHUNK_WIDTH; x++){
				int gx = x + chunk_x * CHUNK_WIDTH;
				int gy = chunk_y * CHUNK_HEIGHT;
				int gz = z + chunk_z * CHUNK_DEPTH;
				int new_chunk_y = chunk_y;

				int light = chunkUpper->light_map->getS(x, 0, z);

				Chunk* current = chunk;
				if (light == 15){
					for (int y = CHUNK_HEIGHT - 1;; y--){
						if (y < 0){
							new_chunk_y--;
							y += CHUNK_HEIGHT;
						}
						if (new_chunk_y != current->chunk_y) current = chunks->getChunk(chunk_x, new_chunk_y, chunk_z);
						if (!current) break;
						voxel* vox = &(current->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]);//chunks->get(gx,gy+y,gz);
						Block* block = Block::blocks[vox->id];
						if (!block->lightPassing) break;
						current->light_map->setS(x, y, z, 15);
						current->needsUpdate = true;
						solverS->add(gx, y + new_chunk_y * CHUNK_HEIGHT, gz);
					}
				} else if (light){
					solverS->add(gx, gy + CHUNK_HEIGHT, gz);
				}
			}
		}
    } else if (sky) {
		for (int z = 0; z < CHUNK_DEPTH; z++){
			for (int x = 0; x < CHUNK_WIDTH; x++){
				int gx = x + chunk_x * CHUNK_WIDTH;
				int gz = z + chunk_z * CHUNK_DEPTH;
				int new_chunk_y = chunk_y;

				Chunk* current = chunk;
				for (int y = CHUNK_HEIGHT - 1;; y--){
					if (y < 0){
						new_chunk_y--;
						y += CHUNK_HEIGHT;
					}
					if (new_chunk_y != current->chunk_y) current = chunks->getChunk(chunk_x, new_chunk_y, chunk_z);
					if (!current) break;
					voxel* vox = &(current->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]);//chunks->get(gx,gy+y,gz);
					Block* block = Block::blocks[vox->id];
					if (!block->lightPassing) break;
					current->light_map->setS(x, y, z, 15);
					current->needsUpdate = true;
					solverS->add(gx, y + new_chunk_y * CHUNK_HEIGHT, gz);
				}
			}
		}
	}

	for (uint y = 0; y < CHUNK_HEIGHT; y++){
		for (uint z = 0; z < CHUNK_DEPTH; z++){
			for (uint x = 0; x < CHUNK_WIDTH; x++){
				voxel vox = chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
				Block* block = Block::blocks[vox.id];
				if (block->emission[0] || block->emission[1] || block->emission[2]){
					int gx = x + chunk_x * CHUNK_WIDTH;
					int gy = y + chunk_y * CHUNK_HEIGHT;
					int gz = z + chunk_z * CHUNK_DEPTH;
					solverR->add(gx,gy,gz,block->emission[0]);
					solverG->add(gx,gy,gz,block->emission[1]);
					solverB->add(gx,gy,gz,block->emission[2]);
				}
			}
		}
	}
	for (int y = -1; y <= CHUNK_HEIGHT; y++){
		for (int z = -1; z <= CHUNK_DEPTH; z++){
			for (int x = -1; x <= CHUNK_WIDTH; x++){
				if (!(x == -1 || x == CHUNK_WIDTH || y == -1 || y == CHUNK_HEIGHT || z == -1 || z == CHUNK_DEPTH)) continue;
				int gx = x + chunk_x * CHUNK_WIDTH;
				int gy = y + chunk_y * CHUNK_HEIGHT;
				int gz = z + chunk_z * CHUNK_DEPTH;
				solverR->add(gx,gy,gz);
				solverG->add(gx,gy,gz);
				solverB->add(gx,gy,gz);
                if (sky) solverS->add(gx,gy,gz);
			}
		}
	}

	solverR->solve();
	solverG->solve();
	solverB->solve();
	solverS->solve();

	Chunk* other;
	other = chunks->getChunk(chunk_x-1,chunk_y,chunk_z); if (other) other->needsUpdate = true;
	other = chunks->getChunk(chunk_x+1,chunk_y,chunk_z); if (other) other->needsUpdate = true;
	other = chunks->getChunk(chunk_x,chunk_y-1,chunk_z); if (other) other->needsUpdate = true;
	other = chunks->getChunk(chunk_x,chunk_y+1,chunk_z); if (other) other->needsUpdate = true;
	other = chunks->getChunk(chunk_x,chunk_y,chunk_z-1); if (other) other->needsUpdate = true;
	other = chunks->getChunk(chunk_x,chunk_y,chunk_z+1); if (other) other->needsUpdate = true;
}

void Lighting::onBlockSet(int x, int y, int z, int id) {
    Block* block = Block::blocks[id];
    if (id == 0) {
        solverR->remove(x, y, z);
        solverG->remove(x, y, z);
        solverB->remove(x, y, z);

        solverR->solve();
        solverG->solve();
        solverB->solve();

        if (chunks->getLight(x, y + 1, z, 3) == 0xF){
            for (int i = y; i >= 0; --i){
                voxel* vox = chunks->getVoxel(x, i, z);
                if ((vox == nullptr || vox->id == 0) && Block::blocks[id]->skyLightPassing) break;
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
        if (!block->skyLightPassing) {
            solverS->remove(x, y, z);
            for (int i = y - 1; i >= 0; --i){
                solverS->remove(x,i,z);
                if (i == 0 || chunks->getVoxel(x, i - 1, z)->id != 0) break;
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
