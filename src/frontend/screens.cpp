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
#include "gui/panels.h"
#include "../util/stringutil.h"
#include "../engine.h"
#include "../logger/Logger.h"
#include "../graphics/Batch2D.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/UVRegion.h"
#include "../graphics/GfxContext.h"
#include "../core_defs.h"
#include "menu.h"
#include "ContentGfxCache.h"
#include "../logic/LevelController.h"

MenuScreen::MenuScreen(Engine* engine_) : Screen(engine_) {
    auto menu = engine->getGUI()->getMenu();

    if (!menu->has("new-world")) menu->add("new-world", create_new_world_panel(engine, menu));
    if (!menu->has("settings")) menu->add("settings", create_settings_panel(engine, menu));
    if (!menu->has("controls")) menu->add("controls", create_controls_panel(engine, menu));
    if (!menu->has("pause")) menu->add("pause", create_pause_panel(engine, menu));

    menu->add("main", create_main_menu_panel(engine, menu));
    menu->reset();
    menu->set("main");

    batch = new Batch2D(1024);
    uicamera = new Camera(glm::vec3(), Window::height);
	uicamera->perspective = false;
	uicamera->flipped = true;
}

MenuScreen::~MenuScreen() {
    delete batch;
    delete uicamera;
}

void MenuScreen::update(float delta) {
}

void MenuScreen::draw(float delta) {
    Window::clear();
    Window::setBgColor(glm::vec3(0.2f));

    uicamera->fov = Window::height;
    ShaderProgram* uishader = engine->getAssets()->getShader("ui");
    if (uishader == nullptr) {
        LOG_CRITICAL("The shader 'ui' could not be found in the assets");
        throw std::runtime_error("The shader 'ui' could not be found in the assets");
    }
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjView());

    batch->begin();
    Texture* menubg = engine->getAssets()->getTexture("menubg");
    if (menubg == nullptr) {
        LOG_CRITICAL("The texture 'menubg' could not be found in the assets");
        throw std::runtime_error("The texture 'menubg' could not be found in the assets");
    }
    batch->texture(menubg);
    batch->rect(0, 0, 
                Window::width, Window::height, 0, 0, 0, 
                UVRegion(0, 0, Window::width/64, Window::height/64), 
                false, false, glm::vec4(1.0f)
            );
    batch->render();
}

static bool backlight;
LevelScreen::LevelScreen(Engine* engine, Level* level) : Screen(engine), level(level) {
    EngineSettings& settings = engine->getSettings();
    controller = new LevelController(settings, level);

    cache = new ContentGfxCache(level->content, engine->getAssets());
    worldRenderer = new WorldRenderer(engine, level, cache);
    hud = new HudRenderer(engine, level, cache, worldRenderer);

    backlight = settings.graphics.backlight;
}

LevelScreen::~LevelScreen() {
    delete controller;
    delete hud;
    delete worldRenderer;
    delete cache;

    LOG_INFO("World saving");
    World* world = level->world;
	world->write(level);

	delete world;
    delete level;
    LOG_INFO("The world has been successfully saved");
}

void LevelScreen::updateHotkeys() {
    if (Events::justPressed(keycode::F3)) level->player->debug = !level->player->debug;

    if (Events::justPressed(keycode::F5)) {
        level->chunks->saveAndClear();
    }
}

void LevelScreen::update(float delta) {
    gui::GUI* gui = engine->getGUI();
    EngineSettings& settings = engine->getSettings();

    bool inputLocked = hud->isPause() || hud->isInventoryOpen() || gui->isFocusCaught();
    if (!gui->isFocusCaught()) updateHotkeys();

    if (settings.graphics.backlight != backlight) {
        level->chunks->saveAndClear();
        backlight = settings.graphics.backlight;
    }

    if (!hud->isPause()) {
        level->world->updateTimers(delta);
    }

    controller->update(delta, !inputLocked, hud->isPause());
    hud->update();
}

void LevelScreen::draw(float deltaTime) {
    Camera* camera = level->player->camera;

    Viewport viewport(Window::width, Window::height);
    GfxContext context(nullptr, viewport, nullptr);
    worldRenderer->draw(context, camera);
    hud->draw(context);
    if (level->player->debug) hud->drawDebug(1 / deltaTime);
}
