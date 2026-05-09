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
    WideStructs,
    Biomes,
    Heightmap,
    Structures
};

struct ChunkPrototype {
    ChunkPrototypeLevel level = ChunkPrototypeLevel::Void;

    std::unique_ptr<const Biome*[]> biomes;
    std::shared_ptr<Heightmap> heightmap;

    std::vector<Placement> placements;
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

    void generateStructuresWide(ChunkPrototype& prototype, int x, int z);

    void generateStructures(ChunkPrototype& prototype, int x, int z);

    void generatePlacements(
        const ChunkPrototype& prototype, voxel* voxels, int x, int z
    );

    void generateBiomes(ChunkPrototype& prototype, int x, int z);

    void generateHeightmap(ChunkPrototype& prototype, int x, int z);

    void placeStructure(
        const StructurePlacement& placement, int priority,
        int chunkX, int chunkZ
    );

    void placeLine(const LinePlacement& line, int priority);

    void generateLine(
        const ChunkPrototype& prototype, 
        const LinePlacement& placement,
        voxel* voxels, 
        int x, int z
    );

    void generateStructure(
        const ChunkPrototype& prototype, 
        const StructurePlacement& placement,
        voxel* voxels, 
        int x, int z
    );

    void generatePlants(
        const ChunkPrototype& prototype,
        float* values,
        voxel* voxels,
        int x,
        int z,
        const Biome** biomes
    );

    void generateLand(
        const ChunkPrototype& prototype,
        float* values,
        voxel* voxels,
        int x,
        int z,
        const Biome** biomes
    );

    void placeStructures(
        const std::vector<Placement>& placements,
        ChunkPrototype& prototype,
        int x, int z
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
};
