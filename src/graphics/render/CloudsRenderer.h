#pragma once

#include <memory>

#include <graphics/render/commons.h>

class ShaderProgram;
class Camera;
class Weather;

class CloudsRenderer final {
public:
    CloudsRenderer();
    ~CloudsRenderer();

    void draw(
        ShaderProgram& shader,
        const Weather& weather,
        float timer,
        float fogFactor,
        const Camera& camera,
        int quality
    );
private:
    std::array<std::unique_ptr<Mesh<ChunkVertex>>, 2> testMeshes;
};
