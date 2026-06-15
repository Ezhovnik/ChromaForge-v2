#include <world/generator/WorldGenerator.h>

#include <cstring>
#include <algorithm>

#include <content/Content.h>
#include <core_content_defs.h>
#include <voxels/Block.h>
#include <util/timeutil.h>
#include <world/generator/Generator.h>
#include <util/timeutil.h>
#include <constants.h>
#include <math/rand.h>
#include <debug/Logger.h>
#include <world/generator/VoxelFragment.h>
#include <util/listutil.h>
#include <math/voxmaths.h>
#include <math/util.h>

static inline constexpr uint MAX_PARAMETERS = 4;
static inline constexpr uint BASIC_PROTOTYPE_LAYERS = 5;

WorldGenerator::WorldGenerator(
    const Generator& def,
    const Content* content,
    uint64_t seed
) : def(def),
    content(content),
    seed(seed),
    surroundMap(0, BASIC_PROTOTYPE_LAYERS + def.wideStructsChunksRadius * 2)
{
    def.script->initialize(seed);

    uint levels = BASIC_PROTOTYPE_LAYERS + def.wideStructsChunksRadius * 2;

    surroundMap = SurroundMap(0, levels);
    LOG_INFO("Total number of prototype levels is {}", levels);

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
    surroundMap.setLevelCallback(def.wideStructsChunksRadius + 1, 
    [this](int const x, int const z) {
        generateStructuresWide(requirePrototype(x, z), x, z);
    });
    surroundMap.setLevelCallback(levels - 3, [this](int const x, int const z) {
        generateBiomes(requirePrototype(x, z), x, z);
    });
    surroundMap.setLevelCallback(levels - 2, [this](int const x, int const z) {
        generateHeightmap(requirePrototype(x, z), x, z);
    });
    surroundMap.setLevelCallback(levels - 1, [this](int const x, int const z) {
        generateStructures(requirePrototype(x, z), x, z);
    });

    for (int i = 0; i < def.structures.size(); ++i) {
        def.structures[i]->fragments[0]->prepare(*content);
        for (int j = 1; j < 4; ++j) {
            def.structures[i]->fragments[j] = def.structures[i]->fragments[j - 1]->rotated(*content);
        }
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

void WorldGenerator::placeStructure(
    const StructurePlacement& placement, int priority,
    int chunkX, int chunkZ
) {
    auto& structure = *def.structures[placement.structure]->fragments[placement.rotation];
    auto absolutePosition = glm::ivec3(chunkX * CHUNK_WIDTH, 0, chunkZ * CHUNK_DEPTH) + placement.position;
    auto size = structure.getSize();
    AABB aabb(absolutePosition, absolutePosition + glm::ivec3(size.x, CHUNK_HEIGHT, size.z));
    for (int lcz = -1; lcz <= 1; ++lcz) {
        for (int lcx = -1; lcx <= 1; ++lcx) {
            const auto& found = prototypes.find({chunkX + lcx, chunkZ + lcz});
            if (found == prototypes.end()) continue;
            auto& otherPrototype = *found->second;
            auto chunkAABB = gen_chunk_aabb(chunkX + lcx, chunkZ + lcz);
            if (chunkAABB.intersect(aabb)) {
                otherPrototype.placements.emplace_back(
                    priority,
                    StructurePlacement {
                        placement.structure,
                        placement.position - glm::ivec3(lcx * CHUNK_WIDTH, 0, lcz * CHUNK_DEPTH),
                        placement.rotation
                    }
                );
            }
        }
    }
}

void WorldGenerator::placeLine(const LinePlacement& line, int priority) {
    AABB aabb(line.a, line.b);
    aabb.fix();
    aabb.a -= line.radius;
    aabb.b += line.radius;
    int cxa = floordiv<CHUNK_WIDTH>(aabb.a.x);
    int cza = floordiv<CHUNK_DEPTH>(aabb.a.z);
    int cxb = floordiv<CHUNK_WIDTH>(aabb.b.x);
    int czb = floordiv<CHUNK_DEPTH>(aabb.b.z);
    for (int cz = cza; cz <= czb; ++cz) {
        for (int cx = cxa; cx <= cxb; ++cx) {
            const auto& found = prototypes.find({cx, cz});
            if (found != prototypes.end()) {
                found->second->placements.emplace_back(priority, line);
            }
        }
    }
}

void WorldGenerator::placeStructures(
    const std::vector<Placement>& placements,
    ChunkPrototype& prototype, 
    int chunkX, 
    int chunkZ
) {
    for (const auto& placement : placements) {
        if (auto sp = std::get_if<StructurePlacement>(&placement.placement)) {
            if (sp->structure < 0 || sp->structure >= def.structures.size()) {
                LOG_ERROR("Invalid structure index {}", sp->structure);
                continue;
            }
            placeStructure(*sp, placement.priority, chunkX, chunkZ);
        } else {
            const auto& line = std::get<LinePlacement>(placement.placement);
            placeLine(line, placement.priority);
        }
    }
}

void WorldGenerator::generateStructuresWide(
    ChunkPrototype& prototype, int chunkX, int chunkZ
) {
    if (prototype.level >= ChunkPrototypeLevel::WideStructs) return;

    auto placements = def.script->placeStructuresWide(
        {chunkX * CHUNK_WIDTH, chunkZ * CHUNK_DEPTH},
        {CHUNK_WIDTH, CHUNK_DEPTH},
        CHUNK_HEIGHT
    );
    placeStructures(placements, prototype, chunkX, chunkZ);

    prototype.level = ChunkPrototypeLevel::WideStructs;
}

void WorldGenerator::generateStructures(
    ChunkPrototype& prototype, int chunkX, int chunkZ
) {
    if (prototype.level >= ChunkPrototypeLevel::Structures) return;

    const auto& biomes = prototype.biomes;
    const auto& heightmap = prototype.heightmap;

    auto placements = def.script->placeStructures(
        {chunkX * CHUNK_WIDTH, chunkZ * CHUNK_DEPTH},
        {CHUNK_WIDTH, CHUNK_DEPTH},
        heightmap,
        CHUNK_HEIGHT
    );
    placeStructures(placements, prototype, chunkX, chunkZ);

    PseudoRandom structsRand;
    structsRand.setSeed(chunkX, chunkZ);

    auto heights = heightmap->getValues();
    for (uint z = 0; z < CHUNK_DEPTH; ++z) {
        for (uint x = 0; x < CHUNK_WIDTH; ++x) {
            float rand = structsRand.randFloat();
            const Biome* biome = biomes[z * CHUNK_WIDTH + x];
            int structureId = biome->structures.choose(rand, -1);
            if (structureId == -1) {
                continue;
            }
            uint8_t rotation = structsRand.randU32() % 4;
            int height = heights[z * CHUNK_WIDTH + x] * CHUNK_HEIGHT;
            if (height < def.seaLevel) {
                continue;
            }
            auto& structure = *def.structures[structureId];
            auto& fragment = *structure.fragments[rotation];
            glm::ivec3 position {x, height - structure.meta.lowering, z};
            position.x -= fragment.getSize().x / 2;
            position.z -= fragment.getSize().z / 2;
            placeStructure(
                StructurePlacement {
                    structureId,
                    position,
                    rotation
                },
                1,
                chunkX, 
                chunkZ
            );
        }
    }

    prototype.level = ChunkPrototypeLevel::Structures;
}

void WorldGenerator::generateBiomes(
    ChunkPrototype& prototype, int chunkX, int chunkZ
) {
    if (prototype.level >= ChunkPrototypeLevel::Biomes) return;

    uint bpd = def.biomesBPD;
    auto biomeParams = def.script->generateParameterMaps(
        {floordiv(chunkX * CHUNK_WIDTH, bpd), floordiv(chunkZ * CHUNK_DEPTH, bpd)},
        {floordiv(CHUNK_WIDTH, bpd) + 1, floordiv(CHUNK_DEPTH, bpd) + 1},
        bpd
    );

    for (auto index : def.heightmapInputs) {
        auto copy = std::make_shared<Heightmap>(*biomeParams[index]);
        copy->resize(
            floordiv(CHUNK_WIDTH, def.heightsBPD) + 1,
            floordiv(CHUNK_DEPTH, def.heightsBPD) + 1,
            def.heightsInterpolation
        );
        prototype.heightmapInputs.push_back(std::move(copy));
    }

    for (const auto& map : biomeParams) {
        map->resize(
            CHUNK_WIDTH + bpd, CHUNK_DEPTH + bpd, def.biomesInterpolation
        );
        map->crop(0, 0, CHUNK_WIDTH, CHUNK_DEPTH);
    }
    const auto& biomes = def.biomes;

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

    uint bpd = def.heightsBPD;
    prototype.heightmap = def.script->generateHeightmap(
        {floordiv(chunkX * CHUNK_WIDTH, bpd), floordiv(chunkZ * CHUNK_DEPTH, bpd)},
        {floordiv(CHUNK_WIDTH, bpd) + 1, floordiv(CHUNK_DEPTH, bpd) + 1},
        bpd,
        prototype.heightmapInputs
    );
    prototype.heightmap->clamp();
    prototype.heightmap->resize(
        CHUNK_WIDTH + bpd, CHUNK_DEPTH + bpd, def.heightsInterpolation
    );
    prototype.heightmap->crop(0, 0, CHUNK_WIDTH, CHUNK_DEPTH);
    prototype.level = ChunkPrototypeLevel::Heightmap;
}

void WorldGenerator::update(int centerX, int centerY, int loadDistance) {
    surroundMap.setCenter(centerX, centerY);
    surroundMap.resize(loadDistance + 2);
    surroundMap.setCenter(centerX, centerY);
}

void WorldGenerator::generatePlants(
    const ChunkPrototype& prototype,
    float* heights,
    voxel* voxels,
    int chunkX,
    int chunkZ,
    const Biome** biomes
) {
    const auto& indices = content->getIndices()->blocks;
    PseudoRandom plantsRand;
    plantsRand.setSeed(chunkX, chunkZ);

    for (uint z = 0; z < CHUNK_DEPTH; ++z) {
        for (uint x = 0; x < CHUNK_WIDTH; ++x) {
            const Biome* biome = biomes[z * CHUNK_WIDTH + x];

            int height = heights[z * CHUNK_WIDTH + x] * CHUNK_HEIGHT;
            height = std::min(std::max(0, height), CHUNK_HEIGHT - 1);

            if (height + 1 > def.seaLevel && height + 1 < CHUNK_HEIGHT) {
                float rand = plantsRand.randFloat();
                blockid_t plant = biome->plants.choose(rand);
                if (plant) {
                    auto& voxel = voxels[vox_index(x, height + 1, z)];
                    if (voxel.id) continue;
                    auto& groundVoxel = voxels[vox_index(x, height, z)];
                    if (indices.get(groundVoxel.id)->rt.solid) {
                        voxel = {plant, {}};
                    }
                }
            }
        }
    }
}

void WorldGenerator::generateLand(
    const ChunkPrototype& prototype,
    float* values,
    voxel* voxels,
    int chunkX,
    int chunkZ,
    const Biome** biomes
) {
    uint seaLevel = def.seaLevel;
    for (uint z = 0; z < CHUNK_DEPTH; ++z) {
        for (uint x = 0; x < CHUNK_WIDTH; ++x) {
            const Biome* biome = biomes[z * CHUNK_WIDTH + x];

            int height = values[z * CHUNK_WIDTH + x] * CHUNK_HEIGHT;
            height = std::max(0, height);

            const auto& groundLayers = biome->groundLayers;
            const auto& seaLayers = biome->seaLayers;

            generate_pole(seaLayers, seaLevel, height, seaLevel, voxels, x, z);
            generate_pole(groundLayers, height, 0, seaLevel, voxels, x, z);
        }
    }
}

void WorldGenerator::generate(voxel* voxels, int chunkX, int chunkZ) {
    surroundMap.completeAt(chunkX, chunkZ);

    const auto& prototype = requirePrototype(chunkX, chunkZ);
    const auto values = prototype.heightmap->getValues();

    uint seaLevel = def.seaLevel;

    std::memset(voxels, 0, sizeof(voxel) * CHUNK_VOLUME);

    const auto& indices = content->getIndices()->blocks;
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

        }
    }
    generatePlacements(prototype, voxels, chunkX, chunkZ);
    generatePlants(prototype, values, voxels, chunkX, chunkZ, biomes);

    for (uint i = 0; i < CHUNK_VOLUME; ++i) {
        blockid_t& id = voxels[i].id;
        if (id == BLOCK_STRUCT_AIR) {
            id = BLOCK_AIR;
        }
#ifndef NDEBUG
        if (indices.get(id) == nullptr) {
            abort();
        }
#endif
    }
}

void WorldGenerator::generatePlacements(
    const ChunkPrototype& prototype, voxel* voxels, int chunkX, int chunkZ
) {
    auto placements = prototype.placements;
    std::stable_sort(
        placements.begin(),
        placements.end(), 
        [](const auto& a, const auto& b) {
            return a.priority < b.priority;
        }
    );
    for (const auto& placement : placements) {
        if (auto structure = std::get_if<StructurePlacement>(&placement.placement)) {
            generateStructure(prototype, *structure, voxels, chunkX, chunkZ);
        } else {
            const auto& line = std::get<LinePlacement>(placement.placement);
            generateLine(prototype, line, voxels, chunkX, chunkZ);
        }
    }
}

void WorldGenerator::generateStructure(
    const ChunkPrototype& prototype, 
    const StructurePlacement& placement,
    voxel* voxels, 
    int chunkX, int chunkZ
) {
    if (placement.structure < 0 || placement.structure >= def.structures.size()) {
        LOG_ERROR("Invalid structure index {}", placement.structure);
        return;
    }
    auto& generatingStructure = def.structures[placement.structure];
    auto& structure = *generatingStructure->fragments[placement.rotation];
    auto& structVoxels = structure.getRuntimeVoxels();
    const auto& offset = placement.position;
    const auto& size = structure.getSize();
    for (int y = 0; y < size.y; ++y) {
        int sy = y + offset.y;
        if (sy < 0 || sy >= CHUNK_HEIGHT) continue;
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

void WorldGenerator::generateLine(
    const ChunkPrototype& prototype, 
    const LinePlacement& line,
    voxel* voxels, 
    int chunkX, int chunkZ
) {
    const auto& indices = content->getIndices()->blocks;
    int cgx = chunkX * CHUNK_WIDTH;
    int cgz = chunkZ * CHUNK_DEPTH;

    int const radius = line.radius;

    auto a = line.a;
    auto b = line.b;

    int minX = std::max(0, std::min(a.x - radius - cgx, b.x - radius - cgx));
    int maxX = std::min(CHUNK_WIDTH, std::max(a.x + radius - cgx, b.x + radius - cgx));

    int minZ = std::max(0, std::min(a.z - radius - cgz, b.z - radius - cgz));
    int maxZ = std::min(CHUNK_DEPTH, std::max(a.z + radius - cgz, b.z + radius - cgz));

    int minY = std::max(0, std::min(a.y - radius, b.y - radius));
    int maxY = std::min(CHUNK_HEIGHT, std::max(a.y + radius, b.y + radius));

    for (int y = minY; y < maxY; ++y) {
        for (int z = minZ; z < maxZ; ++z) {
            for (int x = minX; x < maxX; ++x) {
                int gx = x + cgx;
                int gz = z + cgz;
                glm::ivec3 point {gx, y, gz};
                glm::ivec3 closest = util::closest_point_on_segment(
                    a, b, point
                );
                if (y > 0 && util::distance2(closest, point) <= radius * radius) {
                    auto& voxel = voxels[vox_index(x, y, z)];
                    if (line.block != BLOCK_AIR) {
                        voxel = {line.block, {}};
                        continue;
                    }
                    if (!indices.require(voxel.id).replaceable) {
                        voxel = {line.block, {}};
                    }
                    auto& below = voxels[vox_index(x, y - 1, z)];
                    glm::ivec3 closest2 = util::closest_point_on_segment(
                        a, b, {gx, y - 1, gz}
                    );
                    if (util::distance2(closest2, {gx, y - 1, gz}) > radius * radius) {
                        const auto& def = indices.require(below.id);
                        if (def.rt.surfaceReplacement != below.id) {
                            below = {def.rt.surfaceReplacement, {}};
                        }
                    }
                }
            }
        }
    }
}

WorldGenDebugInfo WorldGenerator::createDebugInfo() const {
    const auto& area = surroundMap.getArea();
    const auto& levels = area.getBuffer();
    auto values = std::make_unique<ubyte[]>(area.getWidth() * area.getDepth());

    for (uint i = 0; i < levels.size(); ++i) {
        values[i] = levels[i];
    }

    return WorldGenDebugInfo {
        area.getOffsetX(),
        area.getOffsetZ(),
        static_cast<uint>(area.getWidth()),
        static_cast<uint>(area.getDepth()),
        std::move(values)
    };
}
