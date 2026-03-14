#include "screens.h"

#include <memory>
#include <filesystem>
#include <sstream>
#include <iomanip>

#include <glm/glm.hpp>

#include "../window/Camera.h"
#include "../window/Events.h"
#include "../window/input.h"
#include "../assets/Assets.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../objects/Player.h"
#include "../logic/ChunksController.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "WorldRenderer.h"
#include "hud.h"
#include "gui/GUI.h"
#include "gui/containers.h"
#include "../util/stringutil.h"
#include "../engine.h"
#include "../logger/Logger.h"
#include "../graphics/Batch2D.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/UVRegion.h"
#include "../graphics/GfxContext.h"
#include "../core_defs.h"
#include "menu/menu.h"
#include "ContentGfxCache.h"
#include "../logic/LevelController.h"
#include "LevelFrontend.h"
#include "../graphics/TextureAnimation.h"
#include "../logic/scripting/scripting_frontend.h"
#include "../logic/scripting/scripting.h"
#include "../audio/audio.h"
#include "../physics/Hitbox.h"

Screen::Screen(Engine* engine) : engine(engine), batch(new Batch2D(1024)) {
}

Screen::~Screen() {
}

MenuScreen::MenuScreen(Engine* engine_) : Screen(engine_) {
    auto menu = engine->getGUI()->getMenu();

    menus::refresh_menus(engine);
    menu->reset();
    menu->setPage("main");

    uicamera.reset(new Camera(glm::vec3(), Window::height));
	uicamera->perspective = false;
	uicamera->flipped = true;
}

MenuScreen::~MenuScreen() {
}

void MenuScreen::update(float delta) {
}

void MenuScreen::draw(float delta) {
    Window::clear();
    Window::setBgColor(glm::vec3(0.2f));

    uicamera->setFov(Window::height);
    ShaderProgram* uishader = engine->getAssets()->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjView());

    batch->begin();
    Texture* menubg = engine->getAssets()->getTexture("gui/menubg");

    uint width = Window::width;
    uint height = Window::height;

    batch->texture(menubg);
    batch->rect(
        0, 0, 
        width, height, 
        0, 0, 
        0, 
        UVRegion(0, 0, width / 64, height / 64), 
        false, false, 
        glm::vec4(1.0f)
    );
    batch->flush();
}

static bool backlight;
LevelScreen::LevelScreen(Engine* engine, Level* level) : Screen(engine) {
    EngineSettings& settings = engine->getSettings();
    backlight = settings.graphics.backlight;
    auto assets = engine->getAssets();
    auto menu = engine->getGUI()->getMenu();
    menu->reset();

    controller = std::make_unique<LevelController>(settings, level);
    levelFrontend = std::make_unique<LevelFrontend>(controller.get(), assets);
    worldRenderer = std::make_unique<WorldRenderer>(engine, levelFrontend.get(), controller->getPlayer());
    hud = std::make_unique<Hud>(engine, levelFrontend.get(), controller->getPlayer());

    animator = std::make_unique<TextureAnimator>();
    animator->addAnimations(assets->getAnimations());

    auto content = level->content;
    for (auto& entry : content->getPacks()) {
        auto pack = entry.second.get();
        const ContentPack& info = pack->getInfo();
        std::filesystem::path scriptFile = info.folder/std::filesystem::path("scripts/hud.lua");
        if (std::filesystem::is_regular_file(scriptFile)) scripting::load_hud_script(pack->getEnvironment()->getId(), info.id, scriptFile);
    }
    scripting::on_frontend_init(hud.get());
}

LevelScreen::~LevelScreen() {
    scripting::on_frontend_close();

    controller->onWorldQuit();
    engine->getPaths()->setWorldFolder(std::filesystem::path());
}

void LevelScreen::onEngineShutdown() {
    controller->saveWorld();
}

void LevelScreen::updateHotkeys() {
    if (Events::justPressed(keycode::F1)) hudVisible = !hudVisible;

    if (Events::justPressed(keycode::F3)) controller->getPlayer()->debug = !controller->getPlayer()->debug;

    if (Events::justPressed(keycode::F5)) controller->getLevel()->chunks->saveAndClear();
}

void LevelScreen::update(float delta) {
    gui::GUI* gui = engine->getGUI();
    EngineSettings& settings = engine->getSettings();

    bool inputLocked = hud->isPause() || hud->isInventoryOpen() || gui->isFocusCaught();
    if (!gui->isFocusCaught()) updateHotkeys();

    auto player = controller->getPlayer();
    auto camera = player->camera;
    audio::set_listener(
        camera->position - camera->dir, 
        player->hitbox->velocity,
        camera->dir, 
        camera->up
    );

    controller->getPlayer()->camera->setFov(glm::radians(settings.camera.fov));

    if (settings.graphics.backlight != backlight) {
        controller->getLevel()->chunks->saveAndClear();
        backlight = settings.graphics.backlight;
    }

    if (!hud->isPause()) {
        controller->getLevel()->world->updateTimers(delta);
        animator->update(delta);
    }

    controller->update(delta, !inputLocked, hud->isPause());
    hud->update(hudVisible);
}

void LevelScreen::draw(float deltaTime) {
    auto camera = controller->getPlayer()->currentCamera;

    Viewport viewport(Window::width, Window::height);
    GfxContext context(nullptr, viewport, batch.get());

    worldRenderer->draw(context, camera.get(), hudVisible);

    if (hudVisible) hud->draw(context);
}

LevelController* LevelScreen::getLevelController() const {
    return controller.get();
}
