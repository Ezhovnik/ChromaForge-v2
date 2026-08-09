#pragma once

#include <array>
#include <memory>
#include <string>

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

    bool setTextures(const std::array<std::string, 6>& faces);

    void update(float deltaTime);
    void draw(ShaderProgram& shader, uint width, uint height) const;

    float getRotationSpeed() const;
    void setRotationSpeed(float speed);

    float getRotation() const;
    void setRotation(float angle);
private:
    static std::unique_ptr<Cubemap> loadCubemap(
        Assets& assets, const std::array<std::string, 6>& faces
    );
    static std::unique_ptr<Mesh<ScreenQuadVertex>> createScreenQuad();

    std::unique_ptr<Cubemap> cubemap;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Mesh<ScreenQuadVertex>> mesh;
    Assets& assets;
    float rotationAngle = 0.0f;
    float rotationSpeed = 0.05f;
};
