#include "GUI.h"

#include <algorithm>
#include <stdexcept>

#include "UINode.h"
#include "panels.h"
#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"
#include "../../window/Events.h"
#include "../../window/input.h"
#include "../../window/Camera.h"
#include "../../graphics/ShaderProgram.h"
#include "../../logger/Logger.h"

using namespace gui;

GUI::GUI() {
    container = new Container(glm::vec2(0, 0), glm::vec2(Window::width, Window::height));

    uicamera = new Camera(glm::vec3(), Window::height);
	uicamera->perspective = false;
	uicamera->flipped = true;

    menu = new PagesControl();
    container->add(menu);
}

GUI::~GUI() {
    delete uicamera;
    delete container;
}

PagesControl* GUI::getMenu() {
    return menu;
}

void GUI::activateMouse(float delta) {
    int mx = Events::x;
    int my = Events::y;

    auto hover = container->getAt(glm::vec2(mx, my), nullptr);
    if (this->hover && this->hover != hover) this->hover->hover(false);
    
    if (hover) {
        hover->hover(true);
        if (Events::scroll) hover->scrolled(Events::scroll);
    }
    this->hover = hover;

    if (Events::justClicked(0)) {
        if (pressed == nullptr && this->hover) {
            pressed = hover;
            pressed->click(this, mx, my);
            if (focus && focus != pressed) focus->defocus();
            if (focus != pressed) {
                focus = pressed;
                focus->focus(this);
            }
        }
        if (this->hover == nullptr && focus) {
            focus->defocus();
            focus = nullptr;
        }
    } else if (pressed) {
        pressed->mouseRelease(this, mx, my);
        pressed = nullptr;
    }
} 

void GUI::activate(float delta) {
    container->size(glm::vec2(Window::width, Window::height));
    container->activate(delta);
    auto prevfocus = focus;

    if (!Events::_cursor_locked) activateMouse(delta);
    
    if (focus) {
        if (Events::justPressed(keycode::ESCAPE)) {
            focus->defocus();
            focus = nullptr;
        } else {
            for (auto codepoint : Events::codepoints) {
                focus->typed(codepoint);
            }
            for (auto key : Events::pressedKeys) {
                focus->keyPressed(key);
            }
            if (!Events::_cursor_locked) {
                if (Events::isClicked(mousecode::BUTTON_1)) {
                    int mx = Events::x;
                    int my = Events::y;
                    focus->mouseMove(this, mx, my);
                }
                if (prevfocus == focus){
                    for (int i = mousecode::BUTTON_1; i < mousecode::BUTTON_1 + 12; ++i) {
                        if (Events::justClicked(i)) focus->clicked(this, i);
                    }
                }
            }
        }
    }
    if (focus && !focus->isfocused()) focus = nullptr;
}

void GUI::draw(Batch2D* batch, Assets* assets) {
    menu->setCoord((Window::size() - menu->size()) / 2.0f);
    uicamera->setFov(Window::height);

	ShaderProgram* uishader = assets->getShader("ui");
    if (uishader == nullptr) {
        LOG_CRITICAL("The shader 'ui' could not be found in the assets");
        throw std::runtime_error("The shader 'ui' could not be found in the assets");
    }
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

void GUI::store(std::string name, std::shared_ptr<UINode> node) {
    storage[name] = node;
}

std::shared_ptr<UINode> GUI::get(std::string name) {
    auto found = storage.find(name);
    if (found == storage.end()) return nullptr;
    return found->second;
}

void GUI::remove(std::string name) {
    storage.erase(name);
}
