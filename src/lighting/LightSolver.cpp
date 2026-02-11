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

	lightentry entry;
	entry.x = x;
	entry.y = y;
	entry.z = z;
	entry.light = bright;
	add_queue.push(entry);

	Chunk* chunk = chunks->getChunkByVoxel(entry.x, entry.y, entry.z);
	chunk->setModified(true);
	chunk->light_map->set(entry.x - chunk->chunk_x * CHUNK_WIDTH, entry.y, entry.z - chunk->chunk_z * CHUNK_DEPTH, channel, entry.light);
}

void LightSolver::add(int x, int y, int z) {
    assert (chunks != nullptr);
	add(x, y, z, chunks->getLight(x, y, z, channel));
}

void LightSolver::remove(int x, int y, int z) {
	Chunk* chunk = chunks->getChunkByVoxel(x, y, z);
	if (chunk == nullptr) return;

	int light = chunk->light_map->get(x - chunk->chunk_x * CHUNK_WIDTH, y, z - chunk->chunk_z * CHUNK_DEPTH, channel);
	if (light == 0) return;

	lightentry entry;
	entry.x = x;
	entry.y = y;
	entry.z = z;
	entry.light = light;
	rem_queue.push(entry);

	chunk->light_map->set(entry.x - chunk->chunk_x * CHUNK_WIDTH, entry.y, entry.z - chunk->chunk_z * CHUNK_DEPTH, channel, 0);
}

void LightSolver::solve(){
	while (!rem_queue.empty()){
		const lightentry entry = rem_queue.front();
		rem_queue.pop();

		for (size_t i = 0; i < 6; ++i) {
			int x = entry.x + LightSolver_Consts::coords[i * 3 + 0];
			int y = entry.y + LightSolver_Consts::coords[i * 3 + 1];
			int z = entry.z + LightSolver_Consts::coords[i * 3 + 2];
			Chunk* chunk = chunks->getChunkByVoxel(x, y, z);
			if (chunk) {
                chunk->setModified(true);
				int light = chunks->getLight(x,y,z, channel);
				if (light != 0 && light == entry.light - 1){
					lightentry nentry;
					nentry.x = x;
					nentry.y = y;
					nentry.z = z;
					nentry.light = light;
					rem_queue.push(nentry);
					chunk->light_map->set(x - chunk->chunk_x * CHUNK_WIDTH, y, z - chunk->chunk_z * CHUNK_DEPTH, channel, 0);
				} else if (light >= entry.light){
					lightentry nentry;
					nentry.x = x;
					nentry.y = y;
					nentry.z = z;
					nentry.light = light;
					add_queue.push(nentry);
				}
			}
		}
	}

    const Block* const* blockDefs = contentIds->getBlockDefs();
	while (!add_queue.empty()){
		const lightentry entry = add_queue.front();
		add_queue.pop();

		if (entry.light <= 1) continue;

		for (size_t i = 0; i < 6; i++) {
			int x = entry.x + LightSolver_Consts::coords[i * 3 + 0];
			int y = entry.y + LightSolver_Consts::coords[i * 3 + 1];
			int z = entry.z + LightSolver_Consts::coords[i * 3 + 2];
			Chunk* chunk = chunks->getChunkByVoxel(x, y, z);
			if (chunk) {
                chunk->setModified(true);
				int light = chunks->getLight(x,y,z, channel);
				voxel* vox = chunks->getVoxel(x,y,z);
                const Block* block = blockDefs[vox->id];
				if (block->lightPassing && light + 2 <= entry.light){
					chunk->light_map->set(x - chunk->chunk_x * CHUNK_WIDTH, y, z - chunk->chunk_z * CHUNK_DEPTH, channel, entry.light - 1);
					lightentry nentry;
					nentry.x = x;
					nentry.y = y;
					nentry.z = z;
					nentry.light = entry.light - 1;
					add_queue.push(nentry);
				}
			}
		}
	}
}
