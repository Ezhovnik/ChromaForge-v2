#pragma once

#include <string>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <math/Heightmap.h>
#include <world/generator/StructurePlacement.h>

class Content;
class VoxelStructure;

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

struct PlantEntry {
    std::string block;
    float weight;

    struct {
        blockid_t id;
    } rt;

    bool operator > (const PlantEntry& other) const {
        return weight > other.weight;
    }
};

struct BiomePlants {
    static inline float MIN_CHANCE = 0.000001f;

    std::vector<PlantEntry> plants;
    float weightsSum;
    float chance;

    inline blockid_t choose(float rand) const {
        if (plants.empty() || rand > chance || chance < MIN_CHANCE) {
            return 0;
        }
        rand = rand / chance;
        rand *= weightsSum;
        for (const auto& plant : plants) {
            rand -= plant.weight;
            if (rand <= 0.0f) {
                return plant.rt.id;
            }
        }
        return plants[plants.size() - 1].rt.id;
    }
};

struct Biome {
    std::string name;
    std::vector<BiomeParameter> parameters;
    BiomePlants plants;
    BlocksLayers groundLayers;
    BlocksLayers seaLayers;
};

class GeneratorScript {
public:
    virtual ~GeneratorScript() = default;

    virtual std::vector<std::shared_ptr<VoxelStructure>> loadStructures() = 0;

    virtual std::shared_ptr<Heightmap> generateHeightmap(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed
    ) = 0;

    virtual std::vector<std::shared_ptr<Heightmap>> generateParameterMaps(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed
    ) = 0;

    virtual std::vector<StructurePlacement> placeStructures(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed,
        const std::shared_ptr<Heightmap>& heightmap
    ) = 0;

    virtual const std::vector<Biome>& getBiomes() const = 0;
    virtual uint getBiomeParameters() const = 0;

    virtual uint getSeaLevel() const = 0;

    virtual void prepare(const Content* content) = 0;
};

struct Generator {
    std::string name;
    std::unique_ptr<GeneratorScript> script;

    Generator(std::string name) : name(std::move(name)) {}
    Generator(const Generator&) = delete;
};
