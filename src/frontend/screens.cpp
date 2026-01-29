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
#include "../graphics/Batch2D.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/UVRegion.h"
#include "../graphics/GfxContext.h"

std::shared_ptr<gui::UINode> create_main_menu_panel(Engine* engine) {
    gui::Panel* panel = new gui::Panel(glm::vec2(400, 200), glm::vec4(5.0f), 1.0f);
    std::shared_ptr<gui::UINode> panelptr(panel);
    panel->color(glm::vec4(0.0f));
	panel->setCoord(glm::vec2(10, 10));

    panel->add((new gui::Button(L"New World", glm::vec4(10.f)))->listenAction([=](gui::GUI* gui) {
        panel->visible(false);
        gui->get("new-world")->visible(true);
    }));
    gui::Panel* worldsPanel = new gui::Panel(glm::vec2(390, 200), glm::vec4(5.0f));
    worldsPanel->color(glm::vec4(0.1f));

    std::filesystem::path saves_folder = engine_fs::get_saves_folder();
    if (std::filesystem::is_directory(saves_folder)) {
        for (auto const& entry : std::filesystem::directory_iterator(saves_folder)) {
            std::string name = entry.path().filename().string();
            gui::Button* button = new gui::Button(util::str2wstr_utf8(name), glm::vec4(10.0f, 8.0f, 10.0f, 8.0f));
            button->color(glm::vec4(0.5f));
            button->listenAction([engine, panel, name](gui::GUI*) {
                LOG_INFO("Loading world");
                EngineSettings& settings = engine->getSettings();
                auto folder = engine_fs::get_saves_folder()/std::filesystem::u8path(name);
                World* world = new World(name, folder, 42, settings);
                auto screen = new LevelScreen(engine, world->load(settings));
                engine->setScreen(std::shared_ptr<Screen>(screen));
                LOG_INFO("The world is loaded");
            });
            worldsPanel->add(button);
        }
    }
    panel->add(worldsPanel);

    panel->add((new gui::Button(L"Settings", glm::vec4(10.f)))->listenAction([=](gui::GUI* gui) {
        panel->visible(false);
        gui->store("back", panelptr);
        gui->get("settings")->visible(true);
    }));
    
    panel->add((new gui::Button(L"Quit", glm::vec4(10.f)))->listenAction([](gui::GUI*) {
        Window::setShouldClose(true);
    }));

    return panelptr;
}

std::shared_ptr<gui::UINode> create_new_world_panel(Engine* engine) {
    gui::Panel* panel = new gui::Panel(glm::vec2(400, 200), glm::vec4(5.0f), 1.0f);
    std::shared_ptr<gui::UINode> panelptr(panel);
    panel->color(glm::vec4(0.0f));
	panel->setCoord(glm::vec2(10, 10));

    gui::TextBox* worldNameInput;
    {
        gui::Label* label = new gui::Label(L"World Name");
        panel->add(label);

        gui::TextBox* input = new gui::TextBox(L"New World", glm::vec4(6.0f));
        panel->add(std::shared_ptr<gui::UINode>(input));
        worldNameInput = input;
    }

    gui::TextBox* seedInput;
    {
        gui::Label* label = new gui::Label(L"Seed");
        panel->add(label);

        uint64_t randseed = rand() ^ (rand() << 8) ^ 
                        (rand() << 16) ^ (rand() << 24) ^
                        ((uint64_t)rand() << 32) ^ 
                        ((uint64_t)rand() << 40) ^
                        ((uint64_t)rand() << 56);

        seedInput = new gui::TextBox(std::to_wstring(randseed), glm::vec4(6.0f));
        panel->add(seedInput);
    }

    {
        gui::Button* button = new gui::Button(L"Create World", glm::vec4(10.0f));
        button->margin(glm::vec4(0, 20, 0, 0));
        glm::vec4 basecolor = worldNameInput->color();   
        button->listenAction([=](gui::GUI*) {
            std::wstring name = worldNameInput->text();
            std::string nameutf8 = util::wstr2str_utf8(name);

            if (!util::is_valid_filename(name) || engine_fs::is_world_name_used(nameutf8)) {
                panel->listenInterval(0.1f, [worldNameInput, basecolor]() {
                    static bool flag = true;
                    if (flag) worldNameInput->color(glm::vec4(0.3f, 0.0f, 0.0f, 0.5f));
                    else worldNameInput->color(basecolor);
                    flag = !flag;
                }, 4);
                return;
            }

            std::wstring seedstr = seedInput->text();
            uint64_t seed;
            if (util::is_integer(seedstr)) {
                try {
                    seed = std::stoull(seedstr);
                } catch (const std::out_of_range& err) {
                    std::hash<std::wstring> hash;
                    seed = hash(seedstr);
                }
            } else {
                std::hash<std::wstring> hash;
                seed = hash(seedstr);
            }
            
            EngineSettings& settings = engine->getSettings();

            auto folder = engine_fs::get_saves_folder()/std::filesystem::u8path(nameutf8);
            std::filesystem::create_directories(folder);
            World* world = new World(nameutf8, folder, seed, settings);
            auto screen = new LevelScreen(engine, world->load(settings));
            engine->setScreen(std::shared_ptr<Screen>(screen));
        });
        panel->add(button);
    }

    panel->add((new gui::Button(L"Back", glm::vec4(10.f)))->listenAction([=](gui::GUI* gui) {
        panel->visible(false);
        gui->get("main-menu")->visible(true);
    }));

    return panelptr;
}

gui::Panel* create_settings_panel(Engine* engine) {
    gui::Panel* panel = new gui::Panel(glm::vec2(400, 200), glm::vec4(5.0f), 1.0f);
    panel->color(glm::vec4(0.0f));
	panel->setCoord(glm::vec2(10, 10));

    {
        panel->add((new gui::Label(L""))->textSupplier([=]() {
            return L"Load Distance: " + std::to_wstring(engine->getSettings().chunks.loadDistance) + L" chunks";
        }));

        gui::TrackBar* trackbar = new gui::TrackBar(3, 66, 10, 1, 3);
        trackbar->supplier([=]() {
            return engine->getSettings().chunks.loadDistance;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().chunks.loadDistance = value;
        });
        panel->add(trackbar);
    }

    {
        panel->add((new gui::Label(L""))->textSupplier([=]() {
            std::wstringstream ss;
            ss << std::fixed << std::setprecision(1);
            ss << engine->getSettings().graphics.fogCurve;
            return L"Fog Curve: " + ss.str();
        }));

        gui::TrackBar* trackbar = new gui::TrackBar(1.0, 6.0, 1.0, 0.1, 2);
        trackbar->supplier([=]() {
            return engine->getSettings().graphics.fogCurve;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().graphics.fogCurve = value;
        });
        panel->add(trackbar);
    }

    {
        gui::Panel* checkpanel = new gui::Panel(glm::vec2(400, 32), glm::vec4(5.0f), 1.0f);
        checkpanel->color(glm::vec4(0.0f));
        checkpanel->orientation(gui::Orientation::horizontal);

        gui::CheckBox* checkbox = new gui::CheckBox();
        checkbox->margin(glm::vec4(0.0f, 0.0f, 5.0f, 0.0f));
        checkbox->supplier([=]() {
            return engine->getSettings().display.swapInterval != 0;
        });
        checkbox->consumer([=](bool checked) {
            engine->getSettings().display.swapInterval = checked;
        });
        checkpanel->add(checkbox);
        checkpanel->add(new gui::Label(L"V-Sync"));

        panel->add(checkpanel);
    }

    panel->add((new gui::Button(L"Back", glm::vec4(10.f)))->listenAction([=](gui::GUI* gui) {
        panel->visible(false);
        gui->get("back")->visible(true);
    }));
    return panel;
}

MenuScreen::MenuScreen(Engine* engine_) : Screen(engine_) {
    gui::GUI* gui = engine->getGUI();

    panel = create_main_menu_panel(engine);
    newWorldPanel = create_new_world_panel(engine);
    newWorldPanel->visible(false);

    auto settingsPanel = std::shared_ptr<gui::UINode>(create_settings_panel(engine));
    settingsPanel->visible(false);

    gui->store("main-menu", panel);
    gui->store("new-world", newWorldPanel);
    if (gui->get("settings") == nullptr) {
        gui->store("settings", settingsPanel);
    }
    gui->add(panel);
    gui->add(newWorldPanel);
    gui->add(settingsPanel);

    batch = new Batch2D(1024);
    uicamera = new Camera(glm::vec3(), Window::height);
	uicamera->perspective = false;
	uicamera->flipped = true;
}

MenuScreen::~MenuScreen() {
    gui::GUI* gui = engine->getGUI();

    gui->remove("main-menu");
    gui->remove("new-world");

    gui->remove(newWorldPanel);
    gui->remove(panel);

    delete batch;
    delete uicamera;
}

void MenuScreen::update(float delta) {
}

void MenuScreen::draw(float delta) {
    panel->setCoord((Window::size() - panel->size()) / 2.0f);
    newWorldPanel->setCoord((Window::size() - newWorldPanel->size()) / 2.0f);
    auto settingsPanel = engine->getGUI()->get("settings");
    settingsPanel->setCoord((Window::size() - settingsPanel->size()) / 2.0f);
    
    Window::clear();
    Window::setBgColor(glm::vec3(0.2f, 0.2f, 0.2f));

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

LevelScreen::LevelScreen(Engine* engine, Level* level) : Screen(engine), level(level) {
    worldRenderer = new WorldRenderer(engine, level);
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
    Camera* camera = level->player->camera;

    Viewport viewport(Window::width, Window::height);
    GfxContext context(nullptr, viewport, nullptr);
    worldRenderer->draw(context, camera, occlusion);
    hud->draw(context);
    if (level->player->debug) hud->drawDebug(1 / deltaTime, occlusion);
}
