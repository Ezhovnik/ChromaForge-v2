#pragma once

#include <memory>

#include <frontend/screens/Screen.h>

class Camera;
class Engine;

class MenuScreen : public Screen {
private:
    std::unique_ptr<Camera> uicamera;
public:
    MenuScreen(Engine& engine);
    ~MenuScreen();

    void onOpen() override;

    void update(float deltaTime) override;
    void draw(float deltaTime) override;

    const char* getName() const override {
        return "menu";
    }
};
