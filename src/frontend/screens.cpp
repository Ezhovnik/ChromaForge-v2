#include "screens.h"

#include <memory>
#include <filesystem>

#include <glm/glm.hpp>

#include "../window/Camera.h"
#include "../window/Events.h"
#include "../window/input.h"
#include "../assets/Assets.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../objects/Player.h"
#include "../voxels/ChunksController.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "world_render.h"
#include "hud.h"
#include "gui/GUI.h"
#include "gui/panels.h"
#include "gui/controls.h"
#include "../util/stringutil.h"
#include "../engine.h"
#include "../logger/Logger.h"
#include "../files/engine_files.h"

MenuScreen::MenuScreen(Engine* engine_) : Screen(engine_) {
    gui::Panel* panel = new gui::Panel(glm::vec2(400, 200), glm::vec4(5.0f), 1.0f);
    panel->color(glm::vec4(0.0f));
	panel->setCoord(glm::vec2(10, 10));

    {
        auto button = new gui::Button(L"Continue", glm::vec4(12.0f, 10.0f, 12.0f, 10.0f));
        button->listenAction([this, panel](gui::GUI*) {
            LOG_INFO("Loading world");
            EngineSettings& settings = engine->getSettings();
            std::filesystem::path folder = engine_fs::get_saves_folder()/std::filesystem::path("world-1");
            World* world = new World("world-1", folder, 42, settings);
            auto screen = new LevelScreen(engine, world->loadLevel(settings));
            engine->setScreen(std::shared_ptr<Screen>(screen));
            LOG_INFO("The world is loaded");
        });
        panel->add(std::shared_ptr<gui::UINode>(button));
    }
    // ATTENTION: FUNCTIONALITY INCOMPLETE ZONE
    /*Panel* worldsPanel = new Panel(vec2(390, 200), vec4(5.0f));
    worldsPanel->color(vec4(0.1f));
    for (auto const& entry : directory_iterator(enginefs::get_worlds_folder())) {
        std::string name = entry.path().filename();
        Button* button = new Button(util::str2wstr_utf8(name), vec4(10.0f, 8.0f, 10.0f, 8.0f));
        button->color(vec4(0.5f));
        button->listenAction([this, panel, name](GUI*) {
            EngineSettings& settings = engine->getSettings();
            World* world = new World(name, enginefs::get_worlds_folder()/name, 42, settings);
            vec3 playerPosition = vec3(0, 64, 0);
            Camera* camera = new Camera(playerPosition, radians(90.0f));
            Player* player = new Player(playerPosition, 4.0f, camera);
            engine->setScreen(new LevelScreen(engine, world->loadLevel(player, settings)));
        });
        worldsPanel->add(shared_ptr<UINode>(button));
    }
    panel->add(shared_ptr<UINode>(worldsPanel));*/
    
    {
        gui::Button* button = new gui::Button(L"Quit", glm::vec4(12.0f, 10.0f, 12.0f, 10.0f));
        button->listenAction([this](gui::GUI*) {
            Window::setShouldClose(true);
        });
        panel->add(std::shared_ptr<gui::UINode>(button));
    }

    this->panel = std::shared_ptr<gui::UINode>(panel);
    engine->getGUI()->add(this->panel);
}

MenuScreen::~MenuScreen() {
    engine->getGUI()->remove(panel);
}

void MenuScreen::update(float delta) {
}

void MenuScreen::draw(float delta) {
    panel->setCoord((Window::size() - panel->size()) / 2.0f);
    
    Window::clear();
    Window::setBgColor(glm::vec3(0.2f, 0.2f, 0.2f));
}

LevelScreen::LevelScreen(Engine* engine, Level* level) : Screen(engine), level(level) {
    worldRenderer = new WorldRenderer(level, engine->getAssets());
    hud = new HudRenderer(engine, level);
}

LevelScreen::~LevelScreen() {
    delete hud;
    delete worldRenderer;

    LOG_INFO("World saving");
    World* world = level->world;
	world->write(level, !engine->getSettings().debug.generatorTestMode);

	delete world;
    delete level;
    LOG_INFO("The world has been successfully saved");
}

void LevelScreen::updateHotkeys() {
    if (Events::justPressed(keycode::O)) occlusion = !occlusion;

    if (Events::justPressed(keycode::F3)) level->player->debug = !level->player->debug;

    if (Events::justPressed(keycode::F5)) {
        for (uint i = 0; i < level->chunks->volume; i++) {
            std::shared_ptr<Chunk> chunk = level->chunks->chunks[i];
            if (chunk != nullptr && chunk->isReady()) {
                chunk->setModified(true);
            }
        }
    }
}

void LevelScreen::update(float delta) {
    gui::GUI* gui = engine->getGUI();
    EngineSettings& settings = engine->getSettings();

    bool inputLocked = hud->isPause() || hud->isInventoryOpen() || gui->isFocusCaught();
    if (!inputLocked) updateHotkeys();

    level->updatePlayer(delta, !inputLocked, hud->isPause(), !inputLocked);
    level->update();
    level->chunksController->update(settings.chunks.loadSpeed);
}

void LevelScreen::draw(float deltaTime) {
    EngineSettings& settings = engine->getSettings();
    Camera* camera = level->player->camera;

    float fogFactor = 18.0f / (float)settings.chunks.loadDistance;
    worldRenderer->draw(camera, occlusion, fogFactor, settings.graphics.fogCurve);
    hud->draw();
    if (level->player->debug) hud->drawDebug(1 / deltaTime, occlusion);
}
