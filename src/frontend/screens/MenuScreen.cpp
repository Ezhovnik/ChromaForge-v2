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
#include <graphics/core/Cubemap.h>
#include <debug/Logger.h>
#include <graphics/core/Mesh.h>
#include <graphics/core/gl_util.h>
#include <assets/Assets.h>
#include <frontend/Panorama.h>
#include <assets/Assets.h>
#include <content/ContentControl.h>

MenuScreen::MenuScreen(Engine& engine) : Screen(engine) {
    engine.getContentControl().resetContent();

    auto menu = engine.getGUI().getMenu();
    menu->reset();
    menu->setPage("main");

    uicamera = std::make_unique<Camera>(glm::vec3(), engine.getWindow().getSize().y);
    uicamera->perspective = false;
    uicamera->flipped = true;
    uicamera->near = -1.0f;
    uicamera->far = 1.0f;

    panorama = std::make_unique<Panorama>(*engine.getAssets());
    if (!panorama->isValid()) {
        panorama.reset();
    }
}

MenuScreen::~MenuScreen() = default;

void MenuScreen::update(float deltaTime) {
    if (panorama) {
        panorama->update(deltaTime);
    }
}

void MenuScreen::draw(float deltaTime) {
    auto assets = engine.getAssets();
    const auto& size = engine.getWindow().getSize();
    uint width = size.x;
    uint height = size.y;

    display::clear();
    display::setBgColor(glm::vec3(0.2f));

    if (panorama) {
        auto* backShader = assets->get<ShaderProgram>("background");
        panorama->draw(*backShader, width, height);
    } else {
        uicamera->setFov(height);
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
}
