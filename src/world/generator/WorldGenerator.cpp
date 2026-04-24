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

static inline void generate_pole(
    const BlocksLayers& layers,
    int height,
    int bottom,
    int seaLevel,
    voxel* voxels,
    int x,
    int z
) {
    uint y = height;
    uint layerExtension = 0;
    for (const auto& layer : layers.layers) {
        if (y < seaLevel && !layer.below_sea_level) {
            layerExtension = std::max(0, layer.height);
            continue;
        }

        int layerHeight = layer.height;
        if (layerHeight == -1) {
            layerHeight = y - layers.lastLayersHeight - bottom + 1;
        } else {
            layerHeight += layerExtension;
        }
        layerHeight = std::min(static_cast<uint>(layerHeight), y);

        for (uint i = 0; i < layerHeight; ++i, --y) {
            voxels[vox_index(x, y, z)].id = layer.rt.id;
        }
        layerExtension = 0;
    }
}

void WorldGenerator::generate(
    voxel* voxels, int chunkX, int chunkZ, uint64_t seed
) {
    timeutil::ScopeLogTimer log(555);
    auto heightmap = def.script->generateHeightmap(
        {chunkX * CHUNK_WIDTH, chunkZ * CHUNK_DEPTH}, {CHUNK_WIDTH, CHUNK_DEPTH}, seed
    );
    auto values = heightmap->getValues();
    const auto& groundLayers = def.script->getGroundLayers();
    const auto& seaLayers = def.script->getSeaLayers();

    uint seaLevel = def.script->getSeaLevel();

    std::memset(voxels, 0, sizeof(voxel) * CHUNK_VOLUME);
    for (uint z = 0; z < CHUNK_DEPTH; ++z) {
        for (uint x = 0; x < CHUNK_WIDTH; ++x) {
            int height = values[z * CHUNK_WIDTH + x] * CHUNK_HEIGHT;
            height = std::max(0, height);

            generate_pole(seaLayers, seaLevel, height, seaLevel, voxels, x, z);
            generate_pole(groundLayers, height, 0, seaLevel, voxels, x, z);
        }
    }
}
