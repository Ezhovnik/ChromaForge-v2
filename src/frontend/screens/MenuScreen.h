#pragma once

#include <memory>

#include <frontend/screens/Screen.h>

class Camera;
class Engine;
class Cubemap;
class Mesh;
class ShaderProgram;
class Assets;

class MenuScreen : public Screen {
private:
    std::unique_ptr<Camera> uicamera;

    std::unique_ptr<Cubemap> panoramaCubemap;
    std::unique_ptr<Camera> panoramaCamera;
    std::unique_ptr<Mesh> panoramaMesh;
    bool hasPanorama = false;
    float rotationAngle = 0.0f;

    static std::unique_ptr<Cubemap> loadPanoramaCubemap(Assets& assets);
public:
    MenuScreen(Engine& engine);
    ~MenuScreen();

    void update(float deltaTime) override;
    void draw(float deltaTime) override;
};
