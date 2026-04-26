#pragma once

#include <string>
#include <memory>
#include <vector>

#include <typedefs.h>
#include <core_content_defs.h>
#include <voxels/voxel.h>
#include <constants.h>

class Content;
struct Generator;
class Heightmap;
struct Biome;

enum class ChunkPrototypeLevel {
    Biomes,
    Heightmap
};

struct ChunkPrototype {
    ChunkPrototypeLevel level;

    std::shared_ptr<Heightmap> heightmap;
    std::vector<const Biome*> biomes;

    ChunkPrototype(
        ChunkPrototypeLevel level,
        std::shared_ptr<Heightmap> heightmap, 
        std::vector<const Biome*> biomes
    ) : level(level),
        heightmap(std::move(heightmap)), 
        biomes(std::move(biomes)) {};
};

// Класс для генерации воксельного мира
class WorldGenerator {
	const Generator& def;
    const Content* content;
    uint64_t seed;

    std::unique_ptr<ChunkPrototype> generatePrototype(int x, int z);

    void generateHeightmap(ChunkPrototype* prototype, int x, int z);
public:
	WorldGenerator(
        const Generator& def,
        const Content* content,
        uint64_t seed
    );

	virtual void generate(voxel* voxels, int x, int z);

    inline static std::string DEFAULT = BUILTIN_CONTENT_NAMESPACE + ":default";
};
