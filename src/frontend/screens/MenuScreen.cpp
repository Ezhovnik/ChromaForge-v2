#include <frontend/screens/MenuScreen.h>

#include <graphics/ui/GUI.h>
#include <graphics/ui/elements/layout/Menu.h>
#include <graphics/core/Batch2D.h>
#include <graphics/core/ShaderProgram.h>
#include <window/Window.h>
#include <window/Camera.h>
#include <engine.h>
#include <graphics/core/Texture.h>
#include <math/UVRegion.h>

MenuScreen::MenuScreen(Engine* engine) : Screen(engine) {
    engine->resetContent();

    auto menu = engine->getGUI()->getMenu();
    menu->reset();
    menu->setPage("main");

    uicamera = std::make_unique<Camera>(glm::vec3(), Window::height);
    uicamera->perspective = false;
    uicamera->flipped = true;
}

MenuScreen::~MenuScreen() {
}

void MenuScreen::update(float deltaTime) {
}

void MenuScreen::draw(float deltaTime) {
    auto assets = engine->getAssets();

    Window::clear();
    Window::setBgColor(glm::vec3(0.2f));

    uicamera->setFov(Window::height);

    ShaderProgram* uishader = assets->get<ShaderProgram>("ui");
    uishader->use();
    uishader->uniformMatrix("u_projview", uicamera->getProjView());

    uint width = Window::width;
    uint height = Window::height;

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
