#pragma once

#include <memory>
#include <glm/glm.hpp>

#include <typedefs.h>

class Assets;
class Cubemap;
class Camera;
template<typename T> class Mesh;
struct ScreenQuadVertex;
class ShaderProgram;

class Panorama {
public:
    explicit Panorama(Assets& assets);
    ~Panorama();

    bool isValid() const {
        return cubemap != nullptr;
    }

    void update(float deltaTime);
    void draw(ShaderProgram& shader, uint width, uint height) const;

private:
    static std::unique_ptr<Cubemap> loadCubemap(Assets& assets);
    static std::unique_ptr<Mesh<ScreenQuadVertex>> createScreenQuad();

    std::unique_ptr<Cubemap> cubemap;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Mesh<ScreenQuadVertex>> mesh;
    float rotationAngle = 0.0f;
    float rotationSpeed = 0.05f;
};
