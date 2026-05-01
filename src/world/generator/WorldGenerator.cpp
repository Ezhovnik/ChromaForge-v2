#include <world/generator/WorldGenerator.h>

#include <cstring>

#include <content/Content.h>
#include <core_content_defs.h>
#include <voxels/Block.h>
#include <util/timeutil.h>
#include <world/generator/Generator.h>
#include <util/timeutil.h>
#include <constants.h>
#include <math/rand.h>
#include <debug/Logger.h>
#include <world/generator/VoxelStructure.h>
#include <util/listutil.h>

static inline constexpr uint MAX_PARAMETERS = 4;
static inline constexpr uint MAX_CHUNK_PROTOTYPE_LEVELS = 5;

WorldGenerator::WorldGenerator(
    const Generator& def,
    const Content* content,
    uint64_t seed
) : def(def), 
    content(content), 
    seed(seed), 
    surroundMap(0, MAX_CHUNK_PROTOTYPE_LEVELS) 
{
    surroundMap.setOutCallback([this](int const x, int const z, int8_t) {
        const auto& found = prototypes.find({x, z});
        if (found == prototypes.end()) {
            LOG_WARN("Unable to remove non-existing chunk prototype");
            return;
        }
        prototypes.erase({x, z});
    });
    surroundMap.setLevelCallback(1, [this](int const x, int const z) {
        if (prototypes.find({x, z}) != prototypes.end()) {
            return;
        }
        prototypes[{x, z}] = generatePrototype(x, z);
    });
    surroundMap.setLevelCallback(2, [this](int const x, int const z) {
        generateBiomes(requirePrototype(x, z), x, z);
    });
    surroundMap.setLevelCallback(3, [this](int const x, int const z) {
        generateHeightmap(requirePrototype(x, z), x, z);
    });
    surroundMap.setLevelCallback(4, [this](int const x, int const z) {
        generateStructures(requirePrototype(x, z), x, z);
    });

    structures = def.script->loadStructures();
    for (auto& structure : structures) {
        structure->prepare(*content);
    }
}

WorldGenerator::~WorldGenerator() {}

ChunkPrototype& WorldGenerator::requirePrototype(int x, int z) {
    const auto& found = prototypes.find({x, z});
    if (found == prototypes.end()) {
        LOG_ERROR("Prototype not found");
        throw std::runtime_error("Prototype not found");
    }
    return *found->second;
}

static inline void generate_pole(
    const BlocksLayers& layers,
    int top, int bottom,
    int seaLevel,
    voxel* voxels,
    int x, int z
) {
    uint y = top;
    uint layerExtension = 0;
    for (const auto& layer : layers.layers) {
        if (y < seaLevel && !layer.belowSeaLevel) {
            layerExtension = std::max(0, layer.height);
            continue;
        }
        int layerHeight = layer.height;
        if (layerHeight == -1) {
            layerHeight = y - layers.lastLayersHeight - bottom + 1;
        } else {
            layerHeight += layerExtension;
        }
        layerHeight = std::min(static_cast<uint>(layerHeight), y + 1);

        for (uint i = 0; i < layerHeight; ++i, --y) {
            voxels[vox_index(x, y, z)].id = layer.rt.id;
        }
        layerExtension = 0;
    }
}

static inline const Biome* choose_biome(
    const std::vector<Biome>& biomes,
    const std::vector<std::shared_ptr<Heightmap>>& maps,
    uint x, uint z
) {
    uint paramsCount = maps.size();
    float params[MAX_PARAMETERS];
    for (uint i = 0; i < paramsCount; ++i) {
        params[i] = maps[i]->getUnchecked(x, z);
    }
    const Biome* chosenBiome = nullptr;
    float chosenScore = std::numeric_limits<float>::infinity();
    for (const auto& biome : biomes) {
        float score = 0.0f;
        for (uint i = 0; i < paramsCount; ++i) {
            score += glm::abs((params[i] - biome.parameters[i].value) / biome.parameters[i].weight);
        }
        if (score < chosenScore) {
            chosenScore = score;
            chosenBiome = &biome;
        }
    }
    return chosenBiome;
}

std::unique_ptr<ChunkPrototype> WorldGenerator::generatePrototype(
    int chunkX, int chunkZ
) {
    return std::make_unique<ChunkPrototype>();
}

inline AABB gen_chunk_aabb(int chunkX, int chunkZ) {
    return AABB(
        {chunkX * CHUNK_WIDTH, 0, chunkZ * CHUNK_DEPTH}, 
        {(chunkX + 1) * CHUNK_WIDTH, 256, (chunkZ + 1) * CHUNK_DEPTH}
    );
}

void WorldGenerator::generateStructures(
    ChunkPrototype& prototype, int chunkX, int chunkZ
) {
    if (prototype.level >= ChunkPrototypeLevel::Structures) {
        return;
    }
    util::concat(prototype.structures, def.script->placeStructures(
        {chunkX * CHUNK_WIDTH, chunkZ * CHUNK_DEPTH}, {CHUNK_WIDTH, CHUNK_DEPTH}, seed,
        prototype.heightmap
    ));

    for (const auto& placement : prototype.structures) {
        const auto& offset = placement.position;
        if (placement.structure < 0 || placement.structure >= structures.size()) {
            LOG_ERROR("Invalid structure index {}", placement.structure);
            continue;
        }
        auto& structure = *structures[placement.structure];
        auto position = glm::ivec3(chunkX * CHUNK_WIDTH, 0, chunkZ * CHUNK_DEPTH) + offset;
        auto size = structure.getSize() + glm::ivec3(0, CHUNK_HEIGHT, 0);
        AABB aabb(position, position + size);
        for (int lcz = -1; lcz <= 1; ++lcz) {
            for (int lcx = -1; lcx <= 1; ++lcx) {
                if (lcx == 0 && lcz == 0) {
                    continue;
                }
                auto& otherPrototype = requirePrototype(
                    chunkX + lcx, chunkZ + lcz
                );
                auto chunkAABB = gen_chunk_aabb(chunkX + lcx, chunkZ + lcz);
                if (chunkAABB.intersect(aabb)) {
                    otherPrototype.structures.emplace_back(
                        placement.structure, 
                        placement.position - glm::ivec3(lcx * CHUNK_WIDTH, 0, lcz * CHUNK_DEPTH)
                    );
                }
            }
        }
    }

    prototype.level = ChunkPrototypeLevel::Structures;
}

void WorldGenerator::generateBiomes(
    ChunkPrototype& prototype, int chunkX, int chunkZ
) {
    if (prototype.level >= ChunkPrototypeLevel::Biomes) {
        return;
    }
    auto biomeParams = def.script->generateParameterMaps(
        {chunkX * CHUNK_WIDTH, chunkZ * CHUNK_DEPTH}, {CHUNK_WIDTH, CHUNK_DEPTH}, seed
    );
    const auto& biomes = def.script->getBiomes();

    auto chunkBiomes = std::make_unique<const Biome*[]>(CHUNK_WIDTH * CHUNK_DEPTH);
    for (uint z = 0; z < CHUNK_DEPTH; ++z) {
        for (uint x = 0; x < CHUNK_WIDTH; ++x) {
            chunkBiomes.get()[z * CHUNK_WIDTH + x] = choose_biome(biomes, biomeParams, x, z);
        }
    }

    prototype.biomes = std::move(chunkBiomes);
    prototype.level = ChunkPrototypeLevel::Biomes;
}

void WorldGenerator::generateHeightmap(
    ChunkPrototype& prototype, int chunkX, int chunkZ
) {
    if (prototype.level >= ChunkPrototypeLevel::Heightmap) return;

    prototype.heightmap = def.script->generateHeightmap(
        {chunkX * CHUNK_WIDTH, chunkZ * CHUNK_DEPTH}, {CHUNK_WIDTH, CHUNK_DEPTH}, seed
    );
    prototype.level = ChunkPrototypeLevel::Heightmap;
}

void WorldGenerator::update(int centerX, int centerY, int loadDistance) {
    surroundMap.setCenter(centerX, centerY);
    surroundMap.resize(loadDistance + 1);
    surroundMap.setCenter(centerX, centerY);
}

void WorldGenerator::generate(voxel* voxels, int chunkX, int chunkZ) {
    surroundMap.completeAt(chunkX, chunkZ);

    const auto& prototype = requirePrototype(chunkX, chunkZ);
    const auto values = prototype.heightmap->getValues();

    uint seaLevel = def.script->getSeaLevel();

    std::memset(voxels, 0, sizeof(voxel) * CHUNK_VOLUME);

    PseudoRandom plantsRand;
    plantsRand.setSeed(chunkX, chunkZ);

    const auto& biomes = prototype.biomes.get();
    for (uint z = 0; z < CHUNK_DEPTH; ++z) {
        for (uint x = 0; x < CHUNK_WIDTH; ++x) {
            const Biome* biome = biomes[z * CHUNK_WIDTH + x];

            int height = values[z * CHUNK_WIDTH + x] * CHUNK_HEIGHT;
            height = std::max(0, height);

            const auto& groundLayers = biome->groundLayers;
            const auto& seaLayers = biome->seaLayers;

            generate_pole(seaLayers, seaLevel, height, seaLevel, voxels, x, z);
            generate_pole(groundLayers, height, 0, seaLevel, voxels, x, z);
            
            if (height + 1 > seaLevel) {
                float rand = (plantsRand.randU32() % RAND_MAX) / static_cast<float>(RAND_MAX);
                blockid_t plant = biome->plants.choose(rand);
                if (plant) {
                    voxels[vox_index(x, height + 1, z)].id = plant;
                }
            }
        }
    }

    for (const auto& placement : prototype.structures) {
        if (placement.structure < 0 || placement.structure >= structures.size()) {
            LOG_ERROR("Invalid structure index {}", placement.structure);
            continue;
        }
        auto& structure = *structures[placement.structure];
        auto& structVoxels = structure.getRuntimeVoxels();
        const auto& offset = placement.position;
        const auto& size = structure.getSize();
        for (int y = 0; y < size.y; ++y) {
            int sy = y + offset.y;
            if (sy < 0 || sy >= CHUNK_HEIGHT) {
                continue;
            }
            for (int z = 0; z < size.z; ++z) {
                int sz = z + offset.z;
                if (sz < 0 || sz >= CHUNK_DEPTH) {
                    continue;
                }
                for (int x = 0; x < size.x; ++x) {
                    int sx = x + offset.x;
                    if (sx < 0 || sx >= CHUNK_WIDTH) {
                        continue;
                    }
                    const auto& structVoxel = structVoxels[vox_index(x, y, z, size.x, size.z)];
                    if (structVoxel.id) {
                        voxels[vox_index(sx, sy, sz)] = structVoxel;
                    }
                }
            }
        }
    }
}
