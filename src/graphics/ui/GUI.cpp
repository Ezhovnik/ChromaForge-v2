#include <graphics/ui/GUI.h>

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <graphics/ui/elements/UINode.h>
#include <graphics/ui/elements/Menu.h>
#include <assets/Assets.h>
#include <graphics/core/Batch2D.h>
#include <window/Events.h>
#include <window/input.h>
#include <window/Camera.h>
#include <window/Window.h>
#include <graphics/core/ShaderProgram.h>
#include <debug/Logger.h>
#include <graphics/core/DrawContext.h>
#include <frontend/UIDocument.h>
#include <core_content_defs.h>
#include <graphics/ui/elements/Label.h>
#include <frontend/locale.h>
#include <graphics/ui/gui_util.h>
#include <graphics/ui/elements/Panel.h>
#include <graphics/core/LineBatch.h>
#include <graphics/core/Font.h>
#include <engine/Engine.h>

using namespace gui;

GUI::GUI(Engine& engine)
    : engine(engine),
    input(engine.getInput()),
    batch2D(std::make_unique<Batch2D>(1024)),
    container(std::make_shared<Container>(*this, glm::vec2(1000)))
{
    container->setId("root");
    uicamera = std::make_unique<Camera>(glm::vec3(), Window::height);

	uicamera->perspective = false;
	uicamera->flipped = true;

    menu = std::make_shared<Menu>(*this);
    menu->setId("menu");
    menu->setZIndex(10);
    container->add(menu);
    container->setScrollable(false);

    tooltip = guiutil::create(
        *this,
        "<container color='#000000A0' interactive='false' z-index='999'>"
            "<label id='tooltip.label' pos='2' autoresize='true' multiline='true' text-wrap='false'></label>"
        "</container>"
    );
    store("tooltip", tooltip);
    store("tooltip.label", UINode::find(tooltip, "tooltip.label"));
    container->add(tooltip);
}

GUI::~GUI() = default;

void GUI::setPageLoader(PageLoaderFunc pageLoader) {
    this->pagesLoader = std::move(pageLoader);
    menu->setPageLoader(this->pagesLoader);
}

PageLoaderFunc GUI::getPagesLoader() {
    return pagesLoader;
}

std::shared_ptr<Menu> GUI::getMenu() {
    return menu;
}

void GUI::resetTooltip() {
    tooltipTimer = 0.0f;
    tooltip->setVisible(false);
}

void GUI::updateTooltip(float deltaTime) {
    const auto& cursor = input.getCursor();
    if (hover == nullptr || !hover->isInside(cursor.pos)) return resetTooltip();

    if (tooltipTimer + deltaTime >= hover->getTooltipDelay()) {
        auto label = std::dynamic_pointer_cast<gui::Label>(get("tooltip.label"));
        const auto& text = hover->getTooltip();
        if (text.empty() && tooltip->isVisible()) return resetTooltip();

        if (label && !text.empty()) {
            tooltip->setVisible(true);
            label->setText(langs::get(text));
            auto size = label->getSize() + glm::vec2(4.0f);
            auto pos = cursor.pos + glm::vec2(10.0f);
            auto rootSize = container->getSize();
            pos.x = glm::min(pos.x, rootSize.x - size.x);
            pos.y = glm::min(pos.y, rootSize.y - size.y);
            tooltip->setSize(size);
            tooltip->setPos(pos);
        }
    }
    tooltipTimer += deltaTime;
}

void GUI::activateMouse(float deltaTime, const CursorState& cursor) {
    float mouseDelta = glm::length(cursor.delta);
    doubleClicked = false;
    doubleClickTimer += deltaTime + mouseDelta * 0.1f;

    auto hover = container->getAt(Events::cursor);
    if (this->hover && this->hover != hover) this->hover->setHover(false);

    if (hover) {
        hover->setHover(true);
        int scroll = input.getScroll();
        if (scroll) {
            hover->scrolled(scroll);
        }
    }
    this->hover = hover;

    if (input.justClicked(mousecode::BUTTON_1)) {
        if (pressed == nullptr && this->hover) {
            pressed = hover;
            if (doubleClickTimer < doubleClickDelay) {
                pressed->doubleClick(cursor.pos.x, cursor.pos.y);
                doubleClicked = true;
            } else {
                pressed->click(cursor.pos.x, cursor.pos.y);
            }
            doubleClickTimer = 0.0f;
            if (focus && focus != pressed) focus->defocus();
            if (focus != pressed) {
                focus = pressed;
                focus->onFocus();
                return;
            }
        }
        if (this->hover == nullptr && focus) {
            focus->defocus();
            focus = nullptr;
        }
    } else if (!input.isClicked(mousecode::BUTTON_1) && pressed) {
        pressed->mouseRelease(cursor.pos.x, cursor.pos.y);
        pressed = nullptr;
    }

    if (hover) {
        for (mousecode code : MOUSECODES_ALL) {
            if (input.justClicked(code)) hover->clicked(code);
        }
    }
} 

void GUI::activateFocused() {
    if (input.justPressed(keycode::ESCAPE)) {
        focus->defocus();
        focus = nullptr;
        return;
    }

    for (auto codepoint : input.getCodepoints()) {
        focus->typed(codepoint);
    }
    for (auto key : input.getPressedKeys()) {
        focus->keyPressed(key);
    }

    const auto& cursor = input.getCursor();
    if (!cursor.locked) {
        if (input.isClicked(mousecode::BUTTON_1) && (input.justClicked(mousecode::BUTTON_1) || cursor.delta.x || cursor.delta.y)) {
            if (!doubleClicked) {
                focus->mouseMove(cursor.pos.x, cursor.pos.y);
            }
        }
    }
}

void GUI::activate(float deltaTime, const Viewport& vp) {
    container->setSize(vp.size());
    container->activate(deltaTime);
    auto prevfocus = focus;

    updateTooltip(deltaTime);

    const auto& cursor = input.getCursor();
    if (!cursor.locked){
        activateMouse(deltaTime, cursor);
    } else {
        if (hover) {
            hover->setHover(false);
            hover = nullptr;
        }
    }

    if (focus) activateFocused();
    if (focus && !focus->isFocused()) focus = nullptr;
}

void GUI::postActivate() {
    while (!postRunnables.empty()) {
        runnable callback = postRunnables.front();
        postRunnables.pop();
        callback();
    }
}

void GUI::draw(const DrawContext& parent_context, const Assets& assets) {
    auto ctx = parent_context.sub(batch2D.get());

    auto& viewport = ctx.getViewport();
    glm::vec2 wsize = viewport.size();

    auto& page = menu->getCurrent();
    if (page.panel) {
        menu->setSize(page.panel->getSize());
        page.panel->refresh();
        if (auto panel = std::dynamic_pointer_cast<gui::Panel>(page.panel)) {
            panel->cropToContent();
        }
    }

    menu->setPos((wsize - menu->getSize()) / 2.0f);
    uicamera->setFov(wsize.y);
    uicamera->setAspectRatio(viewport.getRatio());

	ShaderProgram* uishader = assets.get<ShaderProgram>("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjView());

    batch2D->begin();
    container->draw(ctx, assets);

    if (hover) Window::setCursor(hover->getCursor());

    if (hover && debug) {
        auto pos = hover->calcPos();
        const auto& id = hover->getId();
        if (!id.empty()) {
            auto& font = assets.require<Font>(FONT_DEFAULT);
            auto text = util::str2wstr_utf8(id);
            int width = font.calcWidth(text);
            int height = font.getLineHeight();

            batch2D->untexture();
            batch2D->setColor(0, 0, 0);
            batch2D->rect(pos.x, pos.y, width, height);

            batch2D->resetColor();
            font.draw(*batch2D, text, pos.x, pos.y, nullptr, 0);
        }

        batch2D->untexture();
        auto node = hover->getParent();
        while (node) {
            auto pos = node->calcPos();
            auto size = node->getSize();

            batch2D->setColor(0, 255, 255);
            batch2D->lineRect(pos.x, pos.y, size.x - 1, size.y - 1);

            node = node->getParent();
        }
        auto size = hover->getSize();
        batch2D->setColor(0, 255, 0);
        batch2D->lineRect(pos.x, pos.y, size.x - 1, size.y - 1);
    }
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

void GUI::remove(UINode* panel) noexcept {
    container->remove(panel);
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
    if (focus) focus->onFocus();
}

std::shared_ptr<Container> GUI::getContainer() const {
    return container;
}

void GUI::postRunnable(const runnable& callback) {
    postRunnables.push(callback);
}

void GUI::onAssetsLoad(Assets* assets) {
    assets->store(
        std::make_unique<UIDocument>(
            BUILTIN_CONTENT_NAMESPACE + ":root",
            uidocscript {},
            std::dynamic_pointer_cast<gui::UINode>(container),
            nullptr
        ),
        BUILTIN_CONTENT_NAMESPACE + ":root"
    );
}

void GUI::setDoubleClickDelay(float delay) {
    doubleClickDelay = delay;
}

float GUI::getDoubleClickDelay() const {
    return doubleClickDelay;
}

void GUI::toggleDebug() {
    debug = !debug;
}

const Input& GUI::getInput() const {
    return engine.getInput();
}

Input& GUI::getInput() {
    return engine.getInput();
}
