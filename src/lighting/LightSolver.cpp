#include <lighting/LightSolver.h>

#include <assert.h>

#include <lighting/Lightmap.h>
#include <voxels/Chunks.h>
#include <voxels/Chunk.h>
#include <voxels/voxel.h>
#include <voxels/Block.h>
#include <content/Content.h>

LightSolver::LightSolver(
    const ContentIndices& contentIds,
    Chunks& chunks,
    int channel
) : blockDefs(contentIds.blocks.getDefs()),
    chunks(chunks),
    channel(channel) {}

void LightSolver::add(int x, int y, int z, int bright) {
    if (bright <= 1) return;

    Chunk* chunk = chunks.getChunkByVoxel(x, y, z);
    if (chunk == nullptr) return;

    assert(chunk->lightmap != nullptr);
    auto& lightmap = *chunk->lightmap;

    ubyte light = lightmap.get(
        x - chunk->chunk_x * CHUNK_WIDTH,
        y,
        z - chunk->chunk_z * CHUNK_DEPTH,
        channel
    );
    if (bright < light) return;

    add_queue.push(lightentry{x, y, z, (ubyte)bright});

    chunk->flags.modified = true;
    lightmap.set(
        x - chunk->chunk_x * CHUNK_WIDTH,
        y,
        z - chunk->chunk_z * CHUNK_DEPTH,
        channel,
        bright
    );
}

void LightSolver::add(int x, int y, int z) {
    add(x, y, z, chunks.getLight(x, y, z, channel));
}

void LightSolver::remove(int x, int y, int z) {
    Chunk* chunk = chunks.getChunkByVoxel(x, y, z);
    if (chunk == nullptr) return;

    assert(chunk->lightmap != nullptr);
    auto& lightmap = *chunk->lightmap;

    ubyte light = lightmap.get(
        x - chunk->chunk_x * CHUNK_WIDTH,
        y,
        z - chunk->chunk_z * CHUNK_DEPTH,
        channel
    );
    if (light == 0) return;

    rem_queue.push(lightentry {x, y, z, light});
    lightmap.set(
        x - chunk->chunk_x * CHUNK_WIDTH,
        y,
        z - chunk->chunk_z * CHUNK_DEPTH,
        channel,
        0
    );
}

void LightSolver::solve(Chunk* prevailingChunk) {
    const int coords[] = {
        0, 0, 1,
        0, 0,-1,
        0, 1, 0,
        0,-1, 0,
        1, 0, 0,
        -1, 0, 0
    };

    while (!rem_queue.empty()){
        lightentry entry = std::move(rem_queue.front());
        rem_queue.pop();

        for (int i = 0; i < 6; ++i) {
            int imul3 = i * 3;
            int x = entry.x + coords[imul3];
            int y = entry.y + coords[imul3 + 1];
            int z = entry.z + coords[imul3 + 2];

            Chunk* chunk = prevailingChunk;
            if (chunk == nullptr || !chunk->isBlockInside(x, z)) {
                chunk = chunks.getChunkByVoxel(x,y,z);
                if (chunk == nullptr) continue;
            } else if (y < 0 || y >= CHUNK_HEIGHT) {
                continue;
            }

            int lx = x - chunk->chunk_x * CHUNK_WIDTH;
            int lz = z - chunk->chunk_z * CHUNK_DEPTH;
            chunk->flags.modified = true;

            assert(chunk->lightmap != nullptr);
            auto& lightmap = *chunk->lightmap;

            ubyte light = lightmap.get(lx,y,lz, channel);
            if (light != 0 && light == entry.light - 1) {
                voxel* vox = chunks.getVoxel(x, y, z);
                if (vox && vox->id != 0) {
                    const Block* block = blockDefs[vox->id];
                    if (uint8_t emission = block->emission[channel]) {
                        add_queue.push(lightentry {x, y, z, emission});
                        lightmap.set(lx, y, lz, channel, emission);
                    } else {
                        lightmap.set(lx, y, lz, channel, 0);
                    }
                } else {
                    lightmap.set(lx, y, lz, channel, 0);
                }
                rem_queue.push(lightentry {x, y, z, light});
            } else if (light >= entry.light) {
                add_queue.push(lightentry {x, y, z, light});
            }
        }
    }

    while (!add_queue.empty()) {
        lightentry entry = std::move(add_queue.front());
        add_queue.pop();

        for (int i = 0; i < 6; ++i) {
            int imul3 = i * 3;
            int x = entry.x + coords[imul3];
            int y = entry.y + coords[imul3 + 1];
            int z = entry.z + coords[imul3 + 2];
            Chunk* chunk = prevailingChunk;
            if (chunk == nullptr || !chunk->isBlockInside(x, z)) {
                chunk = chunks.getChunkByVoxel(x,y,z);
                if (chunk == nullptr) continue;
            } else if (y < 0 || y >= CHUNK_HEIGHT) {
                continue;
            }

            assert(chunk->lightmap != nullptr);
            auto& lightmap = *chunk->lightmap;

            int local_x = x - chunk->chunk_x * CHUNK_WIDTH;
            int local_z = z - chunk->chunk_z * CHUNK_DEPTH;

            chunk->flags.modified = true;
            ubyte light = lightmap.get(local_x, y, local_z, channel);
            voxel& vox = chunk->voxels[vox_index(local_x, y, local_z)];
            const Block* block = blockDefs[vox.id];
            if (block->lightPassing && light + 2 <= entry.light){
                lightmap.set(local_x, y, local_z, channel, entry.light - 1);
                add_queue.push(lightentry{x, y, z, ubyte(entry.light - 1)});
            }
        }
    }
}
