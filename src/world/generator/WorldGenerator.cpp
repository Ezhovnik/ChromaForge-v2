#include <world/generator/WorldGenerator.h>

#include <cstring>

#include <content/Content.h>
#include <core_content_defs.h>
#include <voxels/Block.h>
#include <voxels/voxel.h>
#include <world/generator/Generator.h>
#include <util/timeutil.h>
#include <constants.h>

WorldGenerator::WorldGenerator(
    const Generator& def,
    const Content* content
) : def(def), content(content) {}

void WorldGenerator::generate(voxel* voxels, int chunkX, int chunkZ, int seed) {
    auto heightmap = def.script->generateHeightmap(
        {chunkX * CHUNK_WIDTH, chunkZ * CHUNK_DEPTH}, {CHUNK_WIDTH, CHUNK_DEPTH}
    );
    timeutil::ScopeLogTimer log(555);
    auto values = heightmap->getValues();
    const auto& layers = def.script->getLayers();
    uint lastLayersHeight = def.script->getLastLayersHeight();
    auto baseWater = content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":water").rt.id;

    std::memset(voxels, 0, sizeof(voxel) * CHUNK_VOLUME);
    for (uint z = 0; z < CHUNK_DEPTH; ++z) {
        for (uint x = 0; x < CHUNK_WIDTH; ++x) {
            int height = values[z * CHUNK_WIDTH + x] * 255 + 10;
            for (uint y = height + 1; y < 64; ++y) {
                voxels[vox_index(x, y, z)].id = baseWater;
            }

            uint y = height;
            for (const auto& layer : layers) {
                uint layerHeight = layer.height;
                if (layerHeight == -1) {
                    layerHeight = y - lastLayersHeight + 1;
                }
                for (uint i = 0; i < layerHeight; ++i, --y) {
                    voxels[vox_index(x, y, z)].id = layer.rt.id;
                }
            }
        }
    }
}
