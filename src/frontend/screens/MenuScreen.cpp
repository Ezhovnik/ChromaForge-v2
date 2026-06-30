#include <frontend/screens/MenuScreen.h>

#include <graphics/ui/GUI.h>
#include <graphics/ui/elements/Menu.h>
#include <graphics/core/Batch2D.h>
#include <graphics/core/ShaderProgram.h>
#include <window/Window.h>
#include <window/Camera.h>
#include <engine/Engine.h>
#include <graphics/core/Texture.h>
#include <math/UVRegion.h>
#include <assets/Assets.h>

MenuScreen::MenuScreen(Engine& engine) : Screen(engine) {
    engine.resetContent();

    auto menu = engine.getGUI().getMenu();
    menu->reset();
    menu->setPage("main");

    uicamera = std::make_unique<Camera>(glm::vec3(), Window::height);
    uicamera->perspective = false;
    uicamera->flipped = true;
}

MenuScreen::~MenuScreen() = default;

void MenuScreen::update(float deltaTime) {
}

void MenuScreen::draw(float deltaTime) {
    auto assets = engine.getAssets();

    Window::clear();
    Window::setBgColor(glm::vec3(0.2f));

    uint width = Window::width;
    uint height = Window::height;

    uicamera->setFov(Window::height);
    uicamera->setAspectRatio(width / static_cast<float>(height));

    ShaderProgram* uishader = assets->get<ShaderProgram>("ui");
    uishader->use();
    uishader->uniformMatrix("u_projview", uicamera->getProjView());

    auto bg = assets->get<Texture>("gui/menubg");
    batch->begin();
    batch->texture(bg);
    batch->rect(
        0, 0, 
        width, height, 0, 0, 0, 
        UVRegion(0, 0, width / bg->getWidth(), height / bg->getHeight()), 
        false, false, glm::vec4(1.0f)
    );
    batch->flush();
}
