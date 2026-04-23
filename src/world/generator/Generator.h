#pragma once

#include <string>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <math/Heightmap.h>

class Content;

struct BlocksLayer {
    std::string block;
    int height;

    struct {
        blockid_t id;
    } rt;
};

class GeneratorScript {
public:
    virtual ~GeneratorScript() = default;

    virtual std::shared_ptr<Heightmap> generateHeightmap(
        const glm::ivec2& offset, const glm::ivec2& size
    ) = 0;

    virtual const std::vector<BlocksLayer>& getLayers() const = 0;

    virtual uint getLastLayersHeight() const = 0;

    virtual void prepare(const Content* content) = 0;
};

struct Generator {
    std::string name;
    std::unique_ptr<GeneratorScript> script;

    Generator(std::string name) : name(std::move(name)) {}
    Generator(const Generator&) = delete;
};
