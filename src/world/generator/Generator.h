#pragma once

#include <string>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <math/Heightmap.h>

class Content;

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
    float origin;
    float weight;
};

struct Biome {
    std::string name;
    std::vector<BiomeParameter> parameters;
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
