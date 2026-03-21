#include "MenuScreen.h"

#include "../../graphics/ui/GUI.h"
#include "../../graphics/ui/elements/layout/Menu.h"
#include "../../graphics/core/Batch2D.h"
#include "../../graphics/core/ShaderProgram.h"
#include "../../window/Window.h"
#include "../../window/Camera.h"
#include "../../engine.h"

MenuScreen::MenuScreen(Engine* engine_) : Screen(engine_) {
    auto menu = engine->getGUI()->getMenu();
    menu->reset();
    menu->setPage("main");

    uicamera.reset(new Camera(glm::vec3(), Window::height));
    uicamera->perspective = false;
    uicamera->flipped = true;
}

MenuScreen::~MenuScreen() {
}

void MenuScreen::update(float deltaTime) {
}

void MenuScreen::draw(float deltaTime) {
    Window::clear();
    Window::setBgColor(glm::vec3(0.2f));

    uicamera->setFov(Window::height);

    ShaderProgram* uishader = engine->getAssets()->getShader("ui");
    uishader->use();
    uishader->uniformMatrix("u_projview", uicamera->getProjView());

    uint width = Window::width;
    uint height = Window::height;

    batch->begin();
    batch->texture(engine->getAssets()->getTexture("gui/menubg"));
    batch->rect(
        0, 0, 
        width, height, 0, 0, 0, 
        UVRegion(0, 0, width / 64, height / 64), 
        false, false, glm::vec4(1.0f)
    );
    batch->flush();
}
