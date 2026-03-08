#include "LightSolver.h"

#include "LightMap.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"
#include "../content/Content.h"

namespace LightSolver_Consts {
    const int coords[] = {
        0, 0, 1,
        0, 0,-1,
        0, 1, 0,
        0,-1, 0,
        1, 0, 0,
        -1, 0, 0
    };
}


LightSolver::LightSolver(const ContentIndices* contentIds, Chunks* chunks, int channel) : chunks(chunks), channel(channel), contentIds(contentIds) {
}

void LightSolver::add(int x, int y, int z, int bright) {
	if (bright <= 1) return;

	add_queue.push(lightentry{x, y, z, (ubyte)bright});

	Chunk* chunk = chunks->getChunkByVoxel(x, y, z);
	chunk->setModified(true);
	chunk->light_map.set(x - chunk->chunk_x * CHUNK_WIDTH, y, z - chunk->chunk_z * CHUNK_DEPTH, channel, bright);
}

void LightSolver::add(int x, int y, int z) {
    assert (chunks != nullptr);
	add(x, y, z, chunks->getLight(x, y, z, channel));
}

void LightSolver::remove(int x, int y, int z) {
	Chunk* chunk = chunks->getChunkByVoxel(x, y, z);
	if (chunk == nullptr) return;

	ubyte light = chunk->light_map.get(x - chunk->chunk_x * CHUNK_WIDTH, y, z - chunk->chunk_z * CHUNK_DEPTH, channel);
	if (light == 0) return;

	rem_queue.push(lightentry {x, y, z, light});
	chunk->light_map.set(x - chunk->chunk_x * CHUNK_WIDTH, y, z - chunk->chunk_z * CHUNK_DEPTH, channel, 0);
}

void LightSolver::solve(){
	while (!rem_queue.empty()){
		const lightentry entry = rem_queue.front();
		rem_queue.pop();

		for (int i = 0; i < 6; ++i) {
			int x = entry.x + LightSolver_Consts::coords[i * 3 + 0];
			int y = entry.y + LightSolver_Consts::coords[i * 3 + 1];
			int z = entry.z + LightSolver_Consts::coords[i * 3 + 2];
			Chunk* chunk = chunks->getChunkByVoxel(x, y, z);
			if (chunk) {
				int local_x = x - chunk->chunk_x * CHUNK_WIDTH;
				int local_z = z - chunk->chunk_z * CHUNK_DEPTH;

                chunk->setModified(true);
				ubyte light = chunk->light_map.get(local_x, y, local_z, channel);
				if (light != 0 && light == entry.light - 1){
					rem_queue.push(lightentry{x, y, z, light});
					chunk->light_map.set(local_x, y, local_z, channel, 0);
				} else if (light >= entry.light){
					add_queue.push(lightentry{x, y, z, light});
				}
			}
		}
	}

    const Block* const* blockDefs = contentIds->getBlockDefs();
	while (!add_queue.empty()) {
		const lightentry entry = add_queue.front();
		add_queue.pop();

		for (int i = 0; i < 6; ++i) {
			int x = entry.x + LightSolver_Consts::coords[i * 3 + 0];
			int y = entry.y + LightSolver_Consts::coords[i * 3 + 1];
			int z = entry.z + LightSolver_Consts::coords[i * 3 + 2];
			Chunk* chunk = chunks->getChunkByVoxel(x, y, z);
			if (chunk) {
				int local_x = x - chunk->chunk_x * CHUNK_WIDTH;
				int local_z = z - chunk->chunk_z * CHUNK_DEPTH;

                chunk->setModified(true);
				ubyte light = chunk->light_map.get(local_x, y, local_z, channel);
				voxel& vox = chunk->voxels[vox_index(local_x, y, local_z)];
				const Block* block = blockDefs[vox.id];
				if (block->lightPassing && light + 2 <= entry.light){
					chunk->light_map.set(local_x, y, local_z, channel, entry.light - 1);
					add_queue.push(lightentry{x, y, z, (ubyte)(entry.light - 1)});
				}
			}
		}
	}
}
