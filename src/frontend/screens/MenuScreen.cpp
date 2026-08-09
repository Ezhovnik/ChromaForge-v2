#include <frontend/screens/MenuScreen.h>

#include <graphics/ui/GUI.h>
#include <graphics/ui/elements/Menu.h>
#include <graphics/core/ShaderProgram.h>
#include <window/Window.h>
#include <window/Camera.h>
#include <engine/Engine.h>
#include <assets/Assets.h>
#include <frontend/Panorama.h>
#include <content/ContentControl.h>

MenuScreen::MenuScreen(
    Engine& engine
) : Screen(engine),
    uicamera(
        std::make_unique<Camera>(glm::vec3(), engine.getWindow().getSize().y)
    ) {
    uicamera->perspective = false;
    uicamera->flipped = true;
    uicamera->near = -1.0f;
    uicamera->far = 1.0f;
}

MenuScreen::~MenuScreen() = default;

void MenuScreen::onOpen() {
    engine.getContentControl().resetContent({});

    auto menu = engine.getGUI().getMenu();
    menu->reset();
}

void MenuScreen::update(float deltaTime) {
    if (panorama) {
        panorama->update(deltaTime);
    }
}

void MenuScreen::draw(float deltaTime) {
    display::clear();
    display::setBgColor(glm::vec3(0.2f));

    if (panorama) {
        const auto& size = engine.getWindow().getSize();
        auto* backShader = engine.getAssets()->get<ShaderProgram>("background");
        panorama->draw(*backShader, size.x, size.y);
    }
}

bool MenuScreen::hasPanorama() const {
    return panorama != nullptr;
}

bool MenuScreen::createPanorama() {
    panorama = std::make_unique<Panorama>(*engine.getAssets());
    if (!panorama->isValid()) {
        panorama.reset();
        return false;
    }
    return true;
}

void MenuScreen::destroyPanorama() {
    panorama.reset();
}

Panorama* MenuScreen::getPanorama() const {
    return panorama.get();
}
