#include <lighting/LightSolver.h>

#include <lighting/Lightmap.h>
#include <voxels/Chunks.h>
#include <voxels/Chunk.h>
#include <voxels/voxel.h>
#include <voxels/Block.h>
#include <content/Content.h>

namespace LightSolverConsts {
	const int coords[] = {
        0, 0, 1,
        0, 0,-1,
        0, 1, 0,
        0,-1, 0,
        1, 0, 0,
        -1, 0, 0
    };
}

LightSolver::LightSolver(
	const ContentIndices& contentIds,
	Chunks& chunks,
	int channel
) : chunks(chunks),
	channel(channel),
	blockDefs(contentIds.blocks.getDefs()) {
}

void LightSolver::add(int x, int y, int z, int bright) {
	if (bright <= 1) return;

	Chunk* chunk = chunks.getChunkByVoxel(x, y, z);
	if (chunk == nullptr) return;

	ubyte light = chunk->lightmap.get(
		x - chunk->chunk_x * CHUNK_WIDTH,
		y,
		z - chunk->chunk_z * CHUNK_DEPTH,
		channel
	);
    if (bright < light) return;

	add_queue.push(lightentry{x, y, z, (ubyte)bright});

	chunk->flags.modified = true;
	chunk->lightmap.set(x - chunk->chunk_x * CHUNK_WIDTH, y, z - chunk->chunk_z * CHUNK_DEPTH, channel, bright);
}

void LightSolver::add(int x, int y, int z) {
	add(x, y, z, chunks.getLight(x, y, z, channel));
}

void LightSolver::remove(int x, int y, int z) {
	Chunk* chunk = chunks.getChunkByVoxel(x, y, z);
	if (chunk == nullptr) return;

	ubyte light = chunk->lightmap.get(x - chunk->chunk_x * CHUNK_WIDTH, y, z - chunk->chunk_z * CHUNK_DEPTH, channel);
	if (light == 0) return;

	rem_queue.push(lightentry {x, y, z, light});
	chunk->lightmap.set(x - chunk->chunk_x * CHUNK_WIDTH, y, z - chunk->chunk_z * CHUNK_DEPTH, channel, 0);
}

void LightSolver::solve() {
	while (!rem_queue.empty()) {
		const lightentry entry = rem_queue.front();
		rem_queue.pop();

		for (int i = 0; i < 6; ++i) {
			int imul3 = i * 3;
			int x = entry.x + LightSolverConsts::coords[imul3];
			int y = entry.y + LightSolverConsts::coords[imul3 + 1];
			int z = entry.z + LightSolverConsts::coords[imul3 + 2];
			Chunk* chunk = chunks.getChunkByVoxel(x, y, z);
			if (chunk) {
				int local_x = x - chunk->chunk_x * CHUNK_WIDTH;
				int local_z = z - chunk->chunk_z * CHUNK_DEPTH;

                chunk->flags.modified = true;
				ubyte light = chunk->lightmap.get(local_x, y, local_z, channel);
				if (light != 0 && light == entry.light - 1) {
					voxel* vox = chunks.getVoxel(x, y, z);
                    if (vox && vox->id != 0) {
                        const Block* block = blockDefs[vox->id];
                        if (uint8_t emission = block->emission[channel]) {
                            add_queue.push(lightentry {x, y, z, emission});
                            chunk->lightmap.set(local_x, y, local_z, channel, emission);
                        } else {
							chunk->lightmap.set(local_x, y, local_z, channel, 0);
						}
					} else {
						chunk->lightmap.set(local_x, y, local_z, channel, 0);
					}

					rem_queue.push(lightentry{x, y, z, light});
				} else if (light >= entry.light) {
					add_queue.push(lightentry{x, y, z, light});
				}
			}
		}
	}

	while (!add_queue.empty()) {
		const lightentry entry = add_queue.front();
		add_queue.pop();

		for (int i = 0; i < 6; ++i) {
			int imul3 = i * 3;
			int x = entry.x + LightSolverConsts::coords[imul3];
			int y = entry.y + LightSolverConsts::coords[imul3 + 1];
			int z = entry.z + LightSolverConsts::coords[imul3 + 2];
			Chunk* chunk = chunks.getChunkByVoxel(x, y, z);
			if (chunk) {
				int local_x = x - chunk->chunk_x * CHUNK_WIDTH;
				int local_z = z - chunk->chunk_z * CHUNK_DEPTH;

                chunk->flags.modified = true;
				ubyte light = chunk->lightmap.get(local_x, y, local_z, channel);
				voxel& vox = chunk->voxels[vox_index(local_x, y, local_z)];
				const Block* block = blockDefs[vox.id];
				if (block->lightPassing && light + 2 <= entry.light){
					chunk->lightmap.set(local_x, y, local_z, channel, entry.light - 1);
					add_queue.push(lightentry{x, y, z, ubyte(entry.light - 1)});
				}
			}
		}
	}
}
