#pragma once

#include <memory>
#include <vector>
#include <array>

#include <graphics/render/commons.h>

class ShaderProgram;
class Camera;
struct Weather;
class Frustum;

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
    struct Layer {
        int diameter;
        int segmentSize;
        std::vector<std::unique_ptr<Mesh<ChunkVertex>>> meshes;
    };

    std::array<Layer, 2> layers;

    void draw(
        Layer& layer,
        Frustum& frustum,
        ShaderProgram& shader,
        const Camera& camera,
        float timer,
        int layerId
    );
};
