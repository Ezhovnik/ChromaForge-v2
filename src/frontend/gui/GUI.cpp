#include "GUI.h"

#include <iostream>
#include <algorithm>

#include "UINode.h"
#include "panels.h"
#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"
#include "../../graphics/ShaderProgram.h"
#include "../../window/Events.h"
#include "../../window/input.h"
#include "../../window/Camera.h"

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

void GUI::activate(float delta) {
    container->size(glm::vec2(Window::width, Window::height));
    container->activate(delta);

    int mx = Events::x;
    int my = Events::y;

    auto hover = container->getAt(glm::vec2(mx, my), nullptr);
    if (this->hover && this->hover != hover) this->hover->hover(false);
    
    if (hover) hover->hover(true);
    this->hover = hover;

    if (Events::justClicked(0)) {
        if (pressed == nullptr && this->hover) {
            pressed = hover;
            pressed->click(this, mx, my);
            if (focus && focus != pressed) focus->defocus();
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
        if (!focus->isFocused()){
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
            if (Events::isClicked(mousecode::BUTTON_1)) {
                focus->mouseMove(this, mx, my);
            }
        }
    }
}

void GUI::draw(Batch2D* batch, Assets* assets) {
    uicamera->fov = Window::height;

    ShaderProgram* uishader = assets->getShader("ui");
    uishader->use();
    uishader->uniformMatrix("u_projview", uicamera->getProjView());

    container->draw(batch, assets);
}

std::shared_ptr<UINode> GUI::getFocused() const {
    return focus;
}

bool GUI::isFocusCaught() const {
    return focus && focus->isFocusKeeper();
}

void GUI::add(std::shared_ptr<UINode> panel) {
    container->add(panel);
}

void GUI::remove(std::shared_ptr<UINode> panel) {
    container->remove(panel);
}

void GUI::store(std::string name, std::shared_ptr<UINode> node) {
    storage[name] = node;
}

std::shared_ptr<UINode> GUI::get(std::string name) {
    auto found = storage.find(name);
    if (found == storage.end()) return nullptr;
    return found->second;
}

void GUI::remove(std::string name) {
    auto it = storage.find(name);
    if (it == storage.end()) return;
    storage.erase(name);
}
