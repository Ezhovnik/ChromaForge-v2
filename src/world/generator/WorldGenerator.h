#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <array>

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
class VoxelFragment;

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

struct WorldGenDebugInfo {
    int areaOffsetX;
    int areaOffsetZ;
    uint areaWidth;
    uint areaDepth;
    std::unique_ptr<ubyte[]> areaLevels;
};

// Класс для генерации воксельного мира
class WorldGenerator {
	const Generator& def;
    const Content* content;
    uint64_t seed;

    std::unordered_map<glm::ivec2, std::unique_ptr<ChunkPrototype>> prototypes;

    SurroundMap surroundMap;

    std::unique_ptr<ChunkPrototype> generatePrototype(int x, int z);

    ChunkPrototype& requirePrototype(int x, int z);

    void generateStructures(ChunkPrototype& prototype, int x, int z);

    void generateBiomes(ChunkPrototype& prototype, int x, int z);

    void generateHeightmap(ChunkPrototype& prototype, int x, int z);

    void placeStructure(
        const glm::ivec3 offset, size_t structure, uint8_t rotation,
        int chunkX, int chunkZ
    );
public:
	WorldGenerator(
        const Generator& def,
        const Content* content,
        uint64_t seed
    );
    ~WorldGenerator();

    void update(int centerX, int centerY, int loadDistance);

	void generate(voxel* voxels, int x, int z);

    WorldGenDebugInfo createDebugInfo() const;

    inline static std::string DEFAULT = BUILTIN_CONTENT_NAMESPACE + ":default";
};
