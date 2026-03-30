#include "GUI.h"

#include <algorithm>
#include <stdexcept>

#include "elements/UINode.h"
#include "elements/layout/Menu.h"
#include "../../assets/Assets.h"
#include "../../graphics/core/Batch2D.h"
#include "../../window/Events.h"
#include "../../window/input.h"
#include "../../window/Camera.h"
#include "../../window/Window.h"
#include "../../graphics/core/ShaderProgram.h"
#include "../../debug/Logger.h"
#include "../../graphics/core/DrawContext.h"
#include "../../frontend/UIDocument.h"
#include "../../core_content_defs.h"

using namespace gui;

GUI::GUI() {
    container = std::make_shared<Container>(glm::vec2(1000));
    uicamera = std::make_unique<Camera>(glm::vec3(), Window::height);

	uicamera->perspective = false;
	uicamera->flipped = true;

    menu = std::make_shared<Menu>();
    menu->setId("menu");
    container->add(menu);
    container->setScrollable(false);
}

GUI::~GUI() {
}

std::shared_ptr<Menu> GUI::getMenu() {
    return menu;
}

void GUI::activateMouse() {
    auto hover = container->getAt(Events::cursor, nullptr);
    if (this->hover && this->hover != hover) this->hover->setHover(false);

    if (hover) {
        hover->setHover(true);
        if (Events::scroll) hover->scrolled(Events::scroll);
    }
    this->hover = hover;

    if (Events::justClicked(mousecode::BUTTON_1)) {
        if (pressed == nullptr && this->hover) {
            pressed = hover;
            pressed->click(this, Events::cursor.x, Events::cursor.y);
            if (focus && focus != pressed) focus->defocus();
            if (focus != pressed) {
                focus = pressed;
                focus->onFocus(this);
                return;
            }
        }
        if (this->hover == nullptr && focus) {
            focus->defocus();
            focus = nullptr;
        }
    } else if (pressed) {
        pressed->mouseRelease(this, Events::cursor.x, Events::cursor.y);
        pressed = nullptr;
    }

    if (hover) {
        for (mousecode code : MOUSECODES_ALL) {
            if (Events::justClicked(code)) hover->clicked(this, code);
        }
    }
} 

void GUI::activateFocused() {
    if (Events::justPressed(keycode::ESCAPE)) {
        focus->defocus();
        focus = nullptr;
        return;
    }

    for (auto codepoint : Events::codepoints) {
        focus->typed(codepoint);
    }
    for (auto key : Events::pressedKeys) {
        focus->keyPressed(key);
    }

    if (!Events::_cursor_locked) {
        if (Events::isClicked(mousecode::BUTTON_1) && (Events::justClicked(mousecode::BUTTON_1) || Events::delta.x || Events::delta.y)) {
            focus->mouseMove(this, Events::cursor.x, Events::cursor.y);
        }
    }
}

void GUI::activate(float deltaTime, const Viewport& vp) {
    while (!postRunnables.empty()) {
        runnable callback = postRunnables.back();
        postRunnables.pop();
        callback();
    }

    container->setSize(vp.size());
    container->activate(deltaTime);
    auto prevfocus = focus;

    if (!Events::_cursor_locked) activateMouse();

    if (focus) activateFocused();
    if (focus && !focus->isFocused()) focus = nullptr;
}

void GUI::draw(const DrawContext* parent_context, Assets* assets) {
    auto& viewport = parent_context->getViewport();
    glm::vec2 wsize = viewport.size();

    menu->setPos((wsize - menu->getSize()) / 2.0f);
    uicamera->setFov(wsize.y);

	ShaderProgram* uishader = assets->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjView());

    parent_context->getBatch2D()->begin();
    container->draw(parent_context, assets);
}

std::shared_ptr<UINode> GUI::getFocused() const {
    return focus;
}

bool GUI::isFocusCaught() const {
    return focus && focus->isFocuskeeper();
}

void GUI::add(std::shared_ptr<UINode> node) {
    container->add(node);
}

void GUI::remove(std::shared_ptr<UINode> panel) noexcept {
    container->remove(panel);
}

void GUI::store(std::string name, std::shared_ptr<UINode> node) {
    storage[name] = node;
}

std::shared_ptr<UINode> GUI::get(std::string name) noexcept {
    auto found = storage.find(name);
    if (found == storage.end()) return nullptr;
    return found->second;
}

void GUI::remove(std::string name) noexcept {
    storage.erase(name);
}

void GUI::setFocus(std::shared_ptr<UINode> node) {
    if (focus) focus->defocus();

    focus = node;
    if (focus) focus->onFocus(this);
}

std::shared_ptr<Container> GUI::getContainer() const {
    return container;
}

void GUI::postRunnable(runnable callback) {
    postRunnables.push(callback);
}

void GUI::onAssetsLoad(Assets* assets) {
    assets->store(new UIDocument(
        BUILTIN_CONTENT_NAMESPACE + ":root", 
        uidocscript {}, 
        std::dynamic_pointer_cast<gui::UINode>(container), 
        nullptr
    ), BUILTIN_CONTENT_NAMESPACE + ":root");
}
