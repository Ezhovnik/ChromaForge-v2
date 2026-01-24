#include "screens.h"

#include <filesystem>
#include <memory>

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
#include "../graphics/UVRegion.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Batch2D.h"
#include "world_render.h"
#include "hud.h"
#include "gui/GUI.h"
#include "gui/panels.h"
#include "gui/controls.h"
#include "../engine.h"
#include "../files/engine_files.h"
#include "../util/stringutil.h"
#include "../logger/Logger.h"

MenuScreen::MenuScreen(Engine* engine_) : Screen(engine_) {
    gui::Panel* panel = new gui::Panel(glm::vec2(400, 200), glm::vec4(5.0f), 1.0f);
    panel->color(glm::vec4(0.0f));
	panel->setCoord(glm::vec2(10, 10));

    {
        gui::Button* button = new gui::Button(L"New world", glm::vec4(12.0f, 10.0f, 12.0f, 10.0f));
        button->listenAction([this, panel](gui::GUI*) {
            LOG_INFO("Loading world");
            EngineSettings& settings = engine->getSettings();
            std::filesystem::path worldFolder = engine_fs::get_saves_folder()/std::filesystem::path("world-1");
            World* world = new World("world-1", worldFolder, 42, settings);

            auto screen = new LevelScreen(engine, world->load(settings));
            engine->setScreen(std::shared_ptr<Screen>(screen));
        });
        panel->add(std::shared_ptr<gui::UINode>(button));
    }
    // gui::Panel* worldsPanel = new gui::Panel(glm::vec2(390, 200), glm::vec4(5.0f));
    // worldsPanel->color(glm::vec4(0.1f));
    // for (auto const& entry : std::filesystem::directory_iterator(engine_fs::get_saves_folder())) {
    //     std::filesystem::path name = entry.path().filename();
    //     gui::Button* button = new gui::Button(util::str2wstr_utf8(name.string()), glm::vec4(10.0f, 8.0f, 10.0f, 8.0f));
    //     button->color(glm::vec4(0.5f));
    //     button->listenAction([this, panel, name](gui::GUI*) {
    //         EngineSettings& settings = engine->getSettings();
    //         World* world = new World(name.string(), engine_fs::get_saves_folder()/name, 42, settings);
    //         engine->setScreen(std::shared_ptr<LevelScreen>(new LevelScreen(engine, world->load(settings))));
    //     });
    //     worldsPanel->add(std::shared_ptr<gui::UINode>(button));
    // }
    // panel->add(std::shared_ptr<gui::UINode>(worldsPanel));
    
    {
        gui::Button* button = new gui::Button(L"Quit", glm::vec4(12.0f, 10.0f, 12.0f, 10.0f));
        button->listenAction([this](gui::GUI*) {
            Window::setShouldClose(true);
        });
        panel->add(std::shared_ptr<gui::UINode>(button));
    }

    this->panel = std::shared_ptr<gui::UINode>(panel);
    engine->getGUI()->add(this->panel);

    // batch = new Batch2D(1024);
    // uicamera = new Camera(glm::vec3(), Window::height);
    // uicamera->perspective = false;
    // uicamera->flipped = true;
}

MenuScreen::~MenuScreen() {
    engine->getGUI()->remove(panel);
    // delete batch;
    // delete uicamera;
}

void MenuScreen::update(float delta) {
}

void MenuScreen::draw(float delta) {
    panel->setCoord((Window::size() - panel->size()) / 2.0f);
    
    Window::clear();
    Window::setBgColor(glm::vec3(0.2f, 0.2f, 0.2f));

    // uicamera->fov = Window::height;
	// ShaderProgram* uishader = engine->getAssets()->getShader("ui");
	// uishader->use();
	// uishader->uniformMatrix("u_projview", uicamera->getProjView());

    // batch->begin();
    // batch->texture(engine->getAssets()->getTexture("menubg"));
    // batch->rect(0, 0, 
    //             Window::width, Window::height, 0, 0, 0, 
    //             UVRegion(0, 0, Window::width/64, Window::height/64), 
    //             false, false, glm::vec4(1.0f));
    // batch->render();
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

void LevelScreen::updateHotKeys() {
    if (Events::justPressed(keycode::O)) occlusion = !occlusion;

    if (Events::justPressed(keycode::F3)) level->player->debug = !level->player->debug;

    if (Events::justPressed(keycode::F5)) {
        for (uint i = 0; i < level->chunks->volume; i++) {
            std::shared_ptr<Chunk> chunk = level->chunks->chunks[i];
            if (chunk != nullptr && chunk->isReady()) chunk->setModified(true);
        }
    }
}

void LevelScreen::update(float delta) {
    gui::GUI* gui = engine->getGUI();
    EngineSettings& settings = engine->getSettings();

    bool inputLocked = hud->isPause() || hud->isInventoryOpen() || gui->isFocusCaught();
    if (!inputLocked) updateHotKeys();

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
