#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <array>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <math/Heightmap.h>
#include <world/generator/StructurePlacement.h>

class Content;
class VoxelFragment;
struct Generator;

struct VoxelStructureMeta {
    std::string name;
};

struct BlocksLayer {
    std::string block;
    int height;
    bool belowSeaLevel;

    struct {
        blockid_t id;
    } rt;
};

struct BlocksLayers {
    std::vector<BlocksLayer> layers;
    uint lastLayersHeight;
};

struct BiomeParameter {
    float value;
    float weight;
};

struct WeightedEntry {
    std::string name;
    float weight;

    struct {
        size_t id;
    } rt;

    bool operator > (const WeightedEntry& other) const {
        return weight > other.weight;
    }
};

struct BiomeElementList {
    static inline float MIN_CHANCE = 1e-6f;

    std::vector<WeightedEntry> entries;
    float weightsSum = 0.0f;
    float chance;

    BiomeElementList() {}

    BiomeElementList(std::vector<WeightedEntry> entries, float chance) : entries(entries), chance(chance) {
        for (const auto& entry : entries) {
            weightsSum += entry.weight;
        }
    }

    inline size_t choose(float rand, size_t def = 0) const {
        if (entries.empty() || rand > chance || chance < MIN_CHANCE) {
            return def;
        }
        rand = rand / chance;
        rand *= weightsSum;
        for (const auto& entry : entries) {
            rand -= entry.weight;
            if (rand <= 0.0f) {
                return entry.rt.id;
            }
        }
        return entries[entries.size() - 1].rt.id;
    }
};

struct Biome {
    std::string name;
    std::vector<BiomeParameter> parameters;
    BiomeElementList plants;
    BiomeElementList structures;
    BlocksLayers groundLayers;
    BlocksLayers seaLayers;
};

class GeneratorScript {
public:
    virtual ~GeneratorScript() = default;

    virtual std::shared_ptr<Heightmap> generateHeightmap(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed
    ) = 0;

    virtual std::vector<std::shared_ptr<Heightmap>> generateParameterMaps(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed
    ) = 0;

    virtual std::vector<StructurePlacement> placeStructures(
        const Generator& def,
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed,
        const std::shared_ptr<Heightmap>& heightmap
    ) = 0;

    virtual const std::vector<Biome>& getBiomes() const = 0;
    virtual uint getBiomeParameters() const = 0;

    virtual uint getSeaLevel() const = 0;

    virtual void prepare(const Generator& def, const Content* content) = 0;
};

struct GeneratingVoxelStructure {
    VoxelStructureMeta meta;
    std::array<std::unique_ptr<VoxelFragment>, 4> fragments;

    GeneratingVoxelStructure(
        VoxelStructureMeta meta,
        std::unique_ptr<VoxelFragment> structure
    );
};

struct Generator {
    std::string name;
    std::unique_ptr<GeneratorScript> script;

    std::unordered_map<std::string, size_t> structuresIndices;
    std::vector<std::unique_ptr<GeneratingVoxelStructure>> structures;

    Generator(std::string name);
    Generator(const Generator&) = delete;
};
