#include <voxels/FlatWorldGenerator.h>

#include <voxels/voxel.h>
#include <voxels/Chunk.h>
#include <content/Content.h>
#include <core_content_defs.h>

void FlatWorldGenerator::generate(voxel* voxels, int cx, int cz, uint64_t seed) {
    for (int z = 0; z < CHUNK_DEPTH; ++z) {
        for (int x = 0; x < CHUNK_WIDTH; ++x) {
            for (int cur_y = 0; cur_y < CHUNK_HEIGHT; ++cur_y){
                int id = BLOCK_AIR;
                blockstate state {};

                if(cur_y == 2) {
                    id = idBedrock;
                } else if(cur_y == 6) {
                    id = idMoss;
                } else if(cur_y > 2 && cur_y <= 5) {
                    id = idDirt;
                } 

                voxels[(cur_y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].id = id;
                voxels[(cur_y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].state = state;
            }
        }
    }
}
