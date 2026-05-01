#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include <typedefs.h>
#include <core_content_defs.h>
#include <voxels/voxel.h>
#include <constants.h>
#include <world/generator/SurroundMap.h>
#include <world/generator/StructurePlacement.h>

class Content;
struct Generator;
class Heightmap;
struct Biome;
class VoxelStructure;

enum class ChunkPrototypeLevel {
    Void = 0,
    Biomes,
    Heightmap,
    Structures
};

struct ChunkPrototype {
    ChunkPrototypeLevel level = ChunkPrototypeLevel::Void;

    std::unique_ptr<const Biome*[]> biomes;
    std::shared_ptr<Heightmap> heightmap;
    std::vector<StructurePlacement> structures;
};

// Класс для генерации воксельного мира
class WorldGenerator {
	const Generator& def;
    const Content* content;
    uint64_t seed;

    std::unordered_map<glm::ivec2, std::unique_ptr<ChunkPrototype>> prototypes;

    SurroundMap surroundMap;

    std::vector<std::shared_ptr<VoxelStructure>> structures;

    std::unique_ptr<ChunkPrototype> generatePrototype(int x, int z);

    ChunkPrototype& requirePrototype(int x, int z);

    void generateStructures(ChunkPrototype& prototype, int x, int z);

    void generateBiomes(ChunkPrototype& prototype, int x, int z);

    void generateHeightmap(ChunkPrototype& prototype, int x, int z);
public:
	WorldGenerator(
        const Generator& def,
        const Content* content,
        uint64_t seed
    );
    ~WorldGenerator();

    void update(int centerX, int centerY, int loadDistance);

	void generate(voxel* voxels, int x, int z);

    inline static std::string DEFAULT = BUILTIN_CONTENT_NAMESPACE + ":default";
};
