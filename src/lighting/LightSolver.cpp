#include "LightSolver.h"

#include "LightMap.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"

LightSolver::LightSolver(Chunks* chunks, int channel) : chunks(chunks), channel(channel) {
}

void LightSolver::add(int x, int y, int z, int bright) {
	if (bright <= 1)
		return;
	lightentry entry;
	entry.x = x;
	entry.y = y;
	entry.z = z;
	entry.light = bright;
	add_queue.push(entry);

	Chunk* chunk = chunks->getChunkByVoxel(entry.x, entry.y, entry.z);
	chunk->needsUpdate = true;
	chunk->light_map->set(entry.x - chunk->chunk_x * CHUNK_WIDTH, entry.y - chunk->chunk_y * CHUNK_HEIGHT, entry.z - chunk->chunk_z * CHUNK_DEPTH, channel, entry.light);
}

void LightSolver::add(int x, int y, int z) {
	add(x,y,z, chunks->getLight(x,y,z, channel));
}

void LightSolver::remove(int x, int y, int z) {
	Chunk* chunk = chunks->getChunkByVoxel(x, y, z);
	if (chunk == nullptr)
		return;

	int light = chunk->light_map->get(x - chunk->chunk_x * CHUNK_WIDTH, y - chunk->chunk_y * CHUNK_HEIGHT, z - chunk->chunk_z * CHUNK_DEPTH, channel);
	if (light == 0){
		return;
	}

	lightentry entry;
	entry.x = x;
	entry.y = y;
	entry.z = z;
	entry.light = light;
	rem_queue.push(entry);

	chunk->light_map->set(entry.x - chunk->chunk_x * CHUNK_WIDTH, entry.y - chunk->chunk_y * CHUNK_HEIGHT, entry.z - chunk->chunk_z * CHUNK_DEPTH, channel, 0);
}

void LightSolver::solve(){
	const int coords[] = {
			0, 0, 1,
			0, 0,-1,
			0, 1, 0,
			0,-1, 0,
			1, 0, 0,
            -1, 0, 0
	};

	while (!rem_queue.empty()){
		lightentry entry = rem_queue.front();
		rem_queue.pop();

		for (size_t i = 0; i < 6; i++) {
			int x = entry.x + coords[i * 3 + 0];
			int y = entry.y + coords[i * 3 + 1];
			int z = entry.z + coords[i * 3 + 2];
			Chunk* chunk = chunks->getChunkByVoxel(x, y, z);
			if (chunk) {
				int light = chunks->getLight(x,y,z, channel);
				if (light != 0 && light == entry.light - 1){
					lightentry nentry;
					nentry.x = x;
					nentry.y = y;
					nentry.z = z;
					nentry.light = light;
					rem_queue.push(nentry);
					chunk->light_map->set(x-chunk->chunk_x * CHUNK_WIDTH, y-chunk->chunk_y * CHUNK_HEIGHT, z-chunk->chunk_z * CHUNK_DEPTH, channel, 0);
					chunk->needsUpdate = true;
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

	while (!add_queue.empty()){
		lightentry entry = add_queue.front();
		add_queue.pop();

		if (entry.light <= 1)
			continue;

		for (size_t i = 0; i < 6; i++) {
			int x = entry.x + coords[i * 3 + 0];
			int y = entry.y + coords[i * 3 + 1];
			int z = entry.z + coords[i * 3 + 2];
			Chunk* chunk = chunks->getChunkByVoxel(x, y, z);
			if (chunk) {
				int light = chunks->getLight(x,y,z, channel);
				voxel* vox = chunks->getVoxel(x,y,z);
                Block* block = Block::blocks[vox->id];
				if (block->lightPassing && light + 2 <= entry.light){
					chunk->light_map->set(x-chunk->chunk_x * CHUNK_WIDTH, y-chunk->chunk_y * CHUNK_HEIGHT, z-chunk->chunk_z * CHUNK_DEPTH, channel, entry.light-1);
					chunk->needsUpdate = true;
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
