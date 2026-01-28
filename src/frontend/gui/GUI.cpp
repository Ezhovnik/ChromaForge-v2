#include "GUI.h"
#include "UINode.h"
#include "panels.h"

#include <algorithm>

#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"
#include "../../window/Events.h"
#include "../../window/input.h"
#include "../../window/Camera.h"
#include "../../graphics/ShaderProgram.h"

using namespace gui;

GUI::GUI() {
    container = new Container(glm::vec2(0, 0), glm::vec2(Window::width, Window::height));

    uicamera = new Camera(glm::vec3(), Window::height);
	uicamera->perspective = false;
	uicamera->flipped = true;
}

GUI::~GUI() {
    delete uicamera;
    delete container;
}

void GUI::activate(float deltaTime) {
    container->size(glm::vec2(Window::width, Window::height));
    container->activate(deltaTime);

    int mx = Events::x;
    int my = Events::y;

    auto hover = container->getAt(glm::vec2(mx, my), nullptr);
    if (this->hover && this->hover != hover) this->hover->hover(false);

    if (hover) hover->hover(true);

    this->hover = hover;

    if (Events::isClicked(0)) {
        if (pressed == nullptr && this->hover) {
            pressed = hover;
            pressed->click(this, mx, my);
            if (focus) focus->defocus();
            focus = pressed;
        }
        if (this->hover == nullptr && focus) {
            focus->defocus();
            focus = nullptr;
        }
    } else if (pressed) {
        pressed->mouseRelease(this, mx, my);
        pressed = nullptr;
    }
    if (focus) {
        if (!focus->isfocused()){
            focus = nullptr;
        } else if (Events::justPressed(keycode::ESCAPE)) {
            focus->defocus();
            focus = nullptr;
        } else {
            for (auto codepoint : Events::codepoints) {
                focus->typed(codepoint);
            }
            for (auto key : Events::pressedKeys) {
                focus->keyPressed(key);
            }
        }
    }
}

void GUI::draw(Batch2D* batch, Assets* assets) {
    uicamera->fov = Window::height;

	ShaderProgram* uishader = assets->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjection()*uicamera->getView());

    batch->begin();
    container->draw(batch, assets);
}

std::shared_ptr<UINode> GUI::getFocused() const {
    return focus;
}

bool GUI::isFocusCaught() const {
    return focus && focus->isfocuskeeper();
}

void GUI::add(std::shared_ptr<UINode> panel) {
    container->add(panel);
}

void GUI::remove(std::shared_ptr<UINode> panel) {
    container->remove(panel);
}
