#pragma once

#include <memory>
#include <glm/glm.hpp>

#include <typedefs.h>

class Assets;
class Cubemap;
class Camera;
class Mesh;
class ShaderProgram;

class Panorama {
public:
    Panorama(Assets& assets);
    ~Panorama();

    bool isValid() const;

    void update(float deltaTime);
    void draw(ShaderProgram& shader, uint width, uint height) const;
private:
    static std::unique_ptr<Cubemap> loadCubemap(Assets& assets);
    static std::unique_ptr<Mesh> createScreenQuad();

    std::unique_ptr<Cubemap> cubemap;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Mesh> mesh;
    float rotationAngle = 0.0f;
};
