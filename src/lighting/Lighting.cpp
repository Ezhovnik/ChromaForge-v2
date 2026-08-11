#include <lighting/Lighting.h>

#include <memory>
#include <string>

#include <lighting/LightSolver.h>
#include <lighting/Lightmap.h>
#include <voxels/Chunks.h>
#include <voxels/Chunk.h>
#include <voxels/voxel.h>
#include <voxels/Block.h>
#include <core_content_defs.h>
#include <typedefs.h>
#include <constants.h>
#include <content/Content.h>
#include <debug/Logger.h>

static debug::Logger logger("lighting");

Lighting::Lighting(
    const ContentIndices& indices, Chunks& chunks
) : indices(indices),
    chunks(chunks)
{
    solverR = std::make_unique<LightSolver>(indices, chunks, 0);
    solverG = std::make_unique<LightSolver>(indices, chunks, 1);
    solverB = std::make_unique<LightSolver>(indices, chunks, 2);
    solverS = std::make_unique<LightSolver>(indices, chunks, 3);
}

Lighting::~Lighting() = default;

void Lighting::clear() {
    const auto& chunks = this->chunks.getChunks();
    for (size_t index = 0; index < chunks.size(); ++index) {
        auto chunk = chunks[index];
        if (chunk == nullptr) continue;

        auto& lightmap = chunk->lightmap;
        if (lightmap == nullptr) continue;

        std::memset(lightmap->map, 0, sizeof(Lightmap::map));
    }
}

void Lighting::preBuildSkyLight(
    Chunk& chunk, const ContentIndices& indices
) {
    assert(chunk.lightmap != nullptr);
    auto& lightmap = *chunk.lightmap;

    const auto* blockDefs = indices.blocks.getDefs();
    int highestPoint = 0;
    for (int z = 0; z < CHUNK_DEPTH; ++z) {
        for (int x = 0; x < CHUNK_WIDTH; ++x) {
            for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {
                int index = (y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x;
                voxel& vox = chunk.voxels[index];
                const Block* block = blockDefs[vox.id];
                if (!block->skyLightPassing) {
                    if (highestPoint < y) highestPoint = y;
                    break;
                }
                lightmap.setS(x, y, z, 15);
            }
        }
    }
    if (highestPoint < CHUNK_HEIGHT - 1) highestPoint++;
    lightmap.highestPoint = highestPoint;
}

void Lighting::buildSkyLight(int cx, int cz) {
    const auto blockDefs = indices.blocks.getDefs();

    Chunk* chunk = chunks.getChunk(cx, cz);
    if (chunk == nullptr) {
        logger.error() << "Attempted to build sky lights to chunk missing in local matrix";
        return;
    }

    assert(chunk->lightmap != nullptr);
    auto& lightmap = *chunk->lightmap;

    for (int z = 0; z < CHUNK_DEPTH; ++z) {
        int gz = z + cz * CHUNK_DEPTH;
        for (int x = 0; x < CHUNK_WIDTH; ++x) {
            int gx = x + cx * CHUNK_WIDTH;
            for (int y = lightmap.highestPoint; y >= 0; --y){
                while (y > 0 && !blockDefs[chunk->voxels[vox_index(x, y, z)].id]->lightPassing) {
                    --y;
                }
                if (lightmap.getS(x, y, z) != 15) {
                    solverS->add(gx, y + 1, gz);
                    for (; y >= 0; y--){
                        solverS->add(gx + 1, y, gz);
                        solverS->add(gx - 1, y, gz);
                        solverS->add(gx, y, gz + 1);
                        solverS->add(gx, y, gz - 1);
                    }
                }
            }
        }
    }
    solverS->solve();
}

void Lighting::onChunkLoaded(int chunk_x, int chunk_z, bool expand) {
    auto& solverR = *this->solverR;
    auto& solverG = *this->solverG;
    auto& solverB = *this->solverB;
    auto& solverS = *this->solverS;

    auto blockDefs = indices.blocks.getDefs();
    auto chunk = chunks.getChunk(chunk_x, chunk_z);
    if (chunk == nullptr) {
        logger.error() << "Attempted to build lights to chunk missing in local matrix";
        return;
    }

    assert(chunk->lightmap != nullptr);
    auto& lightmap = *chunk->lightmap;

    for (uint y = 0; y < CHUNK_HEIGHT; ++y) {
        for (uint z = 0; z < CHUNK_DEPTH; ++z) {
            for (uint x = 0; x < CHUNK_WIDTH; ++x) {
                const voxel& vox = chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
                const Block* block = blockDefs[vox.id];
                int gx = x + chunk_x * CHUNK_WIDTH;
                int gz = z + chunk_z * CHUNK_DEPTH;
                if (block->rt.emissive){
                    solverR.add(gx, y, gz, block->emission[0]);
                    solverG.add(gx, y, gz, block->emission[1]);
                    solverB.add(gx, y, gz, block->emission[2]);
                }
            }
        }
    }

    if (expand) {
        for (int x = 0; x < CHUNK_WIDTH; x += CHUNK_WIDTH - 1) {
            int gx = x + chunk_x * CHUNK_WIDTH;
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                for (int z = 0; z < CHUNK_DEPTH; ++z) {
                    int gz = z + chunk_z * CHUNK_DEPTH;

                    int rgbs = lightmap.get(x, y, z);
                    if (rgbs) {
                        solverR.add(gx, y, gz, Lightmap::extract(rgbs, 0));
                        solverG.add(gx, y, gz, Lightmap::extract(rgbs, 1));
                        solverB.add(gx, y, gz, Lightmap::extract(rgbs, 2));
                        solverS.add(gx, y, gz, Lightmap::extract(rgbs, 3));
                    }
                }
            }
        }

        for (int z = 0; z < CHUNK_DEPTH; z += CHUNK_DEPTH - 1) {
            int gz = z + chunk_z * CHUNK_DEPTH;
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                for (int x = 0; x < CHUNK_WIDTH; ++x) {
                    int gx = x + chunk_x * CHUNK_WIDTH;
                    int rgbs = lightmap.get(x, y, z);
                    if (rgbs) {
                        solverR.add(gx, y, gz, Lightmap::extract(rgbs, 0));
                        solverG.add(gx, y, gz, Lightmap::extract(rgbs, 1));
                        solverB.add(gx, y, gz, Lightmap::extract(rgbs, 2));
                        solverS.add(gx, y, gz, Lightmap::extract(rgbs, 3));
                    }
                }
            }
        }
    }

    solverR.solve(chunk);
    solverG.solve(chunk);
    solverB.solve(chunk);
    solverS.solve(chunk);
}

void Lighting::onBlockSet(int x, int y, int z, blockid_t id) {
    const auto& block = indices.blocks.require(id);

    auto chunk = chunks.getChunkByVoxel(glm::ivec3{x, y, z});

    if (block.skyLightPassing) {
        if (chunks.getLight(x, y + 1, z, 3) == 0xF) {
            for (int i = y; i >= 0; --i) {
                voxel* vox = chunks.getVoxel(x, i, z);
                if (vox == nullptr || indices.blocks.require(vox->id).skyLightPassing == false) break;
                solverS->add(x, i, z, 0xF);
            }
        }
    } else {
        solverS->remove(x, y, z);
        for (int i = y - 1; i >= 0; --i) {
            voxel* vox = chunks.getVoxel(x, i, z);
            if (vox == nullptr || indices.blocks.require(vox->id).skyLightPassing == false) break;
            solverS->remove(x, i, z);
        }
        solverS->solve();
    }

    solverR->remove(x, y, z);
    solverG->remove(x, y, z);
    solverB->remove(x, y, z);

    solverR->solve(chunk);
    solverG->solve(chunk);
    solverB->solve(chunk);

    static const int coords[] = {
        0, 0, 1,
        0, 0,-1,
        0, 1, 0,
        0,-1, 0,
        1, 0, 0,
        -1, 0, 0
    };
    for (int i = 0; i < 6; ++i) {
        int lx = x + coords[i * 3];
        int ly = y + coords[i * 3 + 1];
        int lz = z + coords[i * 3 + 2];

        solverR->add(lx, ly, lz);
        solverG->add(lx, ly, lz);
        solverB->add(lx, ly, lz);
        solverS->add(lx, ly, lz);
    }

    if (block.emission[0]) {
        solverR->add(x, y, z, block.emission[0]);
    }
    if (block.emission[1]) {
        solverG->add(x, y, z, block.emission[1]);
    }
    if (block.emission[2]) {
        solverB->add(x, y, z, block.emission[2]);
    }
    solverR->solve(chunk);
    solverG->solve(chunk);
    solverB->solve(chunk);
    solverS->solve(chunk);
}
