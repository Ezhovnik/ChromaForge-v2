#include "GUI.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "elements/UINode.h"
#include "elements/layout/Menu.h"
#include "assets/Assets.h"
#include "graphics/core/Batch2D.h"
#include "window/Events.h"
#include "window/input.h"
#include "window/Camera.h"
#include "window/Window.h"
#include "graphics/core/ShaderProgram.h"
#include "debug/Logger.h"
#include "graphics/core/DrawContext.h"
#include "frontend/UIDocument.h"
#include "core_content_defs.h"
#include "elements/display/Label.h"
#include "frontend/locale/langs.h"
#include "gui_util.h"

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

    tooltip = guiutil::create(
        "<container color='#000000A0' interactive='false' z-index='999'>"
            "<label id='tooltip.label' pos='2' autoresize='true'></label>"
        "</container>"
    );
    store("tooltip", tooltip);
    store("tooltip.label", UINode::find(tooltip, "tooltip.label"));
    container->add(tooltip);
}

GUI::~GUI() {
}

std::shared_ptr<Menu> GUI::getMenu() {
    return menu;
}

void GUI::resetTooltip() {
    tooltipTimer = 0.0f;
    tooltip->setVisible(false);
}

void GUI::updateTooltip(float deltaTime) {
    if (hover == nullptr || !hover->isInside(Events::cursor)) return resetTooltip();

    if (tooltipTimer + deltaTime >= hover->getTooltipDelay()) {
        auto label = std::dynamic_pointer_cast<gui::Label>(get("tooltip.label"));
        const auto& text = hover->getTooltip();
        if (text.empty() && tooltip->isVisible()) return resetTooltip();

        if (label && !text.empty()) {
            tooltip->setVisible(true);
            label->setText(langs::get(text));
            auto size = label->getSize() + glm::vec2(4.0f);
            auto pos = Events::cursor + glm::vec2(10.0f);
            auto rootSize = container->getSize();
            pos.x = glm::min(pos.x, rootSize.x - size.x);
            pos.y = glm::min(pos.y, rootSize.y - size.y);
            tooltip->setSize(size);
            tooltip->setPos(pos);
        }
    }
    tooltipTimer += deltaTime;
}

void GUI::activateMouse(float deltaTime) {
    float mouseDelta = glm::length(Events::delta);
    doubleClicked = false;
    doubleClickTimer += deltaTime + mouseDelta * 0.1f;

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
            if (doubleClickTimer < doubleClickDelay) {
                pressed->doubleClick(this, Events::cursor.x, Events::cursor.y);
                doubleClicked = true;
            } else {
                pressed->click(this, Events::cursor.x, Events::cursor.y);
            }
            doubleClickTimer = 0.0f;
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
    } else if (!Events::isClicked(mousecode::BUTTON_1) && pressed) {
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
            if (!doubleClicked) {
                focus->mouseMove(this, Events::cursor.x, Events::cursor.y);
            }
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

    updateTooltip(deltaTime);
    if (!Events::_cursor_locked) {
        activateMouse(deltaTime);
    } else {
        if (hover) {
            hover->setHover(false);
            hover = nullptr;
        }
    }

    if (focus) activateFocused();
    if (focus && !focus->isFocused()) focus = nullptr;
}

void GUI::draw(const DrawContext* parent_context, Assets* assets) {
    auto& viewport = parent_context->getViewport();
    glm::vec2 wsize = viewport.size();

    menu->setPos((wsize - menu->getSize()) / 2.0f);
    uicamera->setFov(wsize.y);

	ShaderProgram* uishader = assets->get<ShaderProgram>("ui");
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
    container->add(std::move(node));
}

void GUI::remove(std::shared_ptr<UINode> panel) noexcept {
    container->remove(std::move(panel));
}

void GUI::store(const std::string& name, std::shared_ptr<UINode> node) {
    storage[name] = std::move(node);
}

std::shared_ptr<UINode> GUI::get(const std::string& name) noexcept {
    auto found = storage.find(name);
    if (found == storage.end()) return nullptr;
    return found->second;
}

void GUI::remove(const std::string& name) noexcept {
    storage.erase(name);
}

void GUI::setFocus(std::shared_ptr<UINode> node) {
    if (focus) focus->defocus();

    focus = std::move(node);
    if (focus) focus->onFocus(this);
}

std::shared_ptr<Container> GUI::getContainer() const {
    return container;
}

void GUI::postRunnable(const runnable& callback) {
    postRunnables.push(callback);
}

void GUI::onAssetsLoad(Assets* assets) {
    assets->store(std::make_unique<UIDocument>(
        BUILTIN_CONTENT_NAMESPACE + ":root", 
        uidocscript {}, 
        std::dynamic_pointer_cast<gui::UINode>(container), 
        nullptr
    ), BUILTIN_CONTENT_NAMESPACE + ":root");
}

void GUI::setDoubleClickDelay(float delay) {
    doubleClickDelay = delay;
}

float GUI::getDoubleClickDelay() const {
    return doubleClickDelay;
}
