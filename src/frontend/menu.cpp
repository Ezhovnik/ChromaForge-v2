#include "menu.h"

#include <string>
#include <memory>
#include <filesystem>
#include <random>
#include <chrono>

#include <glm/glm.hpp>

#include "gui/GUI.h"
#include "gui/panels.h"
#include "gui/controls.h"
#include "screens.h"
#include "../util/stringutil.h"
#include "../files/engine_paths.h"
#include "../world/World.h"
#include "../window/Events.h"
#include "../window/Window.h"
#include "../engine.h"
#include "../settings.h"
#include "gui/gui_util.h"
#include "../content/Content.h"
#include "../content/ContentLUT.h"
#include "../files/WorldConverter.h"
#include "../logger/Logger.h"
#include "locale/langs.h"
#include "../content/ContentPack.h"
#include "../definitions.h"
#include "../engine.h"
#include "../content/ContentLUT.h"
#include "../math/rand.h"
#include "../files/WorldFiles.h"
#include "../world/Level.h"

using namespace gui;

inline uint64_t str2seed(std::wstring seedstr) {
    if (util::is_integer(seedstr)) {
        try {
            return std::stoull(seedstr);
        } catch (const std::out_of_range& err) {
            std::hash<std::wstring> hash;
            return hash(seedstr);
        }
    } else {
        std::hash<std::wstring> hash;
        return hash(seedstr);
    }
}

std::shared_ptr<Panel> create_page(Engine* engine, std::string name, int width, float opacity, int interval) {
    PagesControl* menu = engine->getGUI()->getMenu();
    Panel* panel = new Panel(glm::vec2(width, 200), glm::vec4(8.0f), interval);
    panel->color(glm::vec4(0.0f, 0.0f, 0.0f, opacity));

    std::shared_ptr<Panel> ptr(panel);
    menu->add(name, ptr);
    return ptr;
}

Button* create_button(std::wstring text, glm::vec4 padding, glm::vec4 margin, onaction action) {
    auto btn = new Button(langs::get(text, L"menu"), padding, margin);
    btn->listenAction(action);
    return btn;
}

void show_content_missing(Engine* engine, const Content* content, ContentLUT* lut) {
    auto* gui = engine->getGUI();
    auto* menu = gui->getMenu();
    auto panel = create_page(engine, "missing-content", 500, 0.5f, 8);

    panel->add(new Label(langs::get(L"menu.missing-content")));

    Panel* subpanel = new Panel(glm::vec2(500, 100));
    subpanel->color(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));

    for (size_t i = 0; i < lut->countBlocks(); ++i) {
        if (lut->getBlockId(i) == BLOCK_VOID) {
            auto name = lut->getBlockName(i);
            Panel* hpanel = new Panel(glm::vec2(500, 30));
            hpanel->color(glm::vec4(0.0f));
            hpanel->orientation(Orientation::horizontal);

            Label* namelabel = new Label(util::str2wstr_utf8(name));
            namelabel->color(glm::vec4(1.0f, 0.2f, 0.2f, 0.5f));

            Label* typelabel = new Label(L"[block]");
            typelabel->color(glm::vec4(0.5f));
            hpanel->add(typelabel);
            hpanel->add(namelabel);
            subpanel->add(hpanel);
        }
    }
    subpanel->maxLength(400);
    panel->add(subpanel);

    panel->add((new Button(langs::get(L"Back to Main Menu", L"menu"), glm::vec4(8.0f)))->listenAction([=](GUI*){menu->back();}));
    menu->set("missing-content");
}

void show_convert_request(Engine* engine, const Content* content, ContentLUT* lut, std::filesystem::path folder) {
    guiutil::confirm(engine->getGUI(), langs::get(L"world.convert-request"), [=]() {
        LOG_INFO("Convert the world: {}", folder.string());
        auto converter = std::make_unique<WorldConverter>(folder, content, lut);
        while (converter->hasNext()) {
            converter->convertNext();
        }
        converter->write();
        delete lut;
    }, L"", langs::get(L"Cancel"));
}

void open_world(std::string name, Engine* engine) {
    auto paths = engine->getPaths();
    auto folder = paths->getWorldsFolder()/std::filesystem::u8path(name);

    auto& packs = engine->getContentPacks();
    packs.clear();
    try {
        auto packNames = ContentPack::worldPacksList(folder);
        ContentPack::readPacks(paths, packs, packNames, folder);
    } catch (contentpack_error& error) {
        guiutil::alert(engine->getGUI(), langs::get(L"error.pack-not-found") + L": " + util::str2wstr_utf8(error.getPackId()));
        return;
    }
    engine->loadContent();

    auto* content = engine->getContent();
    auto& settings = engine->getSettings();
    std::filesystem::create_directories(folder);
    ContentLUT* lut = World::checkIndices(folder, content);
    if (lut) {
        if (lut->hasMissingContent()) {
            show_content_missing(engine, content, lut);
            delete lut;
        } else {
            show_convert_request(engine, content, lut, folder);
        }
    } else {
        int version;
        {
            WorldFiles wfile(folder, settings.debug);
            version = wfile.getVoxelRegionsVersion();
        }
        if (version != REGION_FORMAT_VERSION) {
            show_convert_request(engine, content, lut, folder);
        } else {
            Level* level = World::load(folder, settings, content, packs);
            engine->setScreen(std::make_shared<LevelScreen>(engine, level));
        }
    }
}

Panel* create_worlds_panel(Engine* engine) {
    auto panel = new Panel(glm::vec2(390, 200), glm::vec4(5.0f));
    panel->color(glm::vec4(1.0f, 1.0f, 1.0f, 0.07f));
    panel->maxLength(400);

    auto paths = engine->getPaths();
    std::filesystem::path worldsFolder = paths->getWorldsFolder();
    if (std::filesystem::is_directory(worldsFolder)) {
        for (auto entry : std::filesystem::directory_iterator(worldsFolder)) {
            if (!entry.is_directory()) continue;

            auto name = entry.path().filename().u8string();
            auto btn = new Button(util::str2wstr_utf8(name), glm::vec4(10.0f, 8.0f, 10.0f, 8.0f));
            btn->color(glm::vec4(1.0f, 1.0f, 1.0f, 0.1f));
            btn->listenAction([=](GUI*) {open_world(name, engine);});
            panel->add(btn);
        }
    }
    return panel;
}

void create_main_menu_panel(Engine* engine, PagesControl* menu) {
    auto panel = create_page(engine, "main", 400, 0.0f, 1);

    panel->add(guiutil::gotoButton(L"New World", "new-world", menu));
    panel->add(create_worlds_panel(engine));
    panel->add(guiutil::gotoButton(L"Settings", "settings", menu));
    panel->add(guiutil::gotoButton(L"Content", "content", menu));

    panel->add((new Button(langs::get(L"Quit", L"menu"), glm::vec4(10.f)))
    ->listenAction([](GUI* gui) {
        Window::setShouldClose(true);
    }));
}

void create_content_panel(Engine* engine, PagesControl* menu) {
    auto panel = create_page(engine, "content", 400, 0.0f, 5);
    panel->add(new Label(L"work in progress"));
    panel->add(guiutil::backButton(menu));
}

void create_new_world_panel(Engine* engine, PagesControl* menu) {
    auto panel = create_page(engine, "new-world", 400, 0.0f, 1);

    TextBox* worldNameInput;
    {
        Label* label = new Label(langs::get(L"Name", L"world"));
        panel->add(label);

        TextBox* input = new TextBox(L"New World", glm::vec4(6.0f));
        panel->add(input);
        worldNameInput = input;
    }

    TextBox* seedInput;
    {
        Label* label = new Label(langs::get(L"Seed", L"world"));
        panel->add(std::shared_ptr<UINode>(label));

        uint64_t randseed = RandomGenerator::get<uint64_t>();

        seedInput = new TextBox(std::to_wstring(randseed), glm::vec4(6.0f));
        panel->add(seedInput);
    }

    glm::vec4 basecolor = worldNameInput->color();
    panel->add(create_button(L"Create World", glm::vec4(10), glm::vec4(1, 20, 1, 1), 
    [=](GUI*) {
        std::wstring name = worldNameInput->text();
        std::string nameutf8 = util::wstr2str_utf8(name);
        EnginePaths* paths = engine->getPaths();

        if (!util::is_valid_filename(name) || paths->isWorldNameUsed(nameutf8)) {
            panel->listenInterval(0.1f, [worldNameInput, basecolor]() {
                static bool flag = true;
                if (flag) worldNameInput->color(glm::vec4(0.3f, 0.0f, 0.0f, 0.5f));
                else worldNameInput->color(basecolor);
                flag = !flag;
            }, 4);
            return;
        }

        std::wstring seedstr = seedInput->text();
        uint64_t seed = str2seed(seedstr);

        worldNameInput->cleanInput();
        seedInput->cleanInput();

        auto folder = paths->getWorldsFolder()/std::filesystem::u8path(nameutf8);
        std::filesystem::create_directories(folder);

        engine->loadAllPacks();
        engine->loadContent();

        Level* level = World::create(nameutf8, folder, seed, engine->getSettings(), engine->getContent(), engine->getContentPacks());
        engine->setScreen(std::make_shared<LevelScreen>(engine, level));
    }));
    panel->add(guiutil::backButton(menu));
}

void create_controls_panel(Engine* engine, PagesControl* menu) {
    auto panel = create_page(engine, "controls", 400, 0.0f, 1);

    {
        panel->add((new Label(L""))->textSupplier([=]() {
            std::wstringstream ss;
            ss << std::fixed << std::setprecision(1);
            ss << engine->getSettings().camera.sensitivity;
            return langs::get(L"Mouse Sensitivity", L"settings") + L": " + ss.str();
        }));

        TrackBar* trackbar = new TrackBar(0.1, 10.0, 2.0, 0.1, 4);
        trackbar->supplier([=]() {
            return engine->getSettings().camera.sensitivity;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().camera.sensitivity = value;
        });
        panel->add(trackbar);
    }

    {
        Panel* scrollPanel = new Panel(glm::vec2(400, 200), glm::vec4(2.0f), 1.0f);
        scrollPanel->color(glm::vec4(0.0f, 0.0f, 0.0f, 0.3f));
        scrollPanel->maxLength(400);

        for (auto& entry : Events::bindings){
            std::string bindname = entry.first;

            Panel* subpanel = new Panel(glm::vec2(400, 40), glm::vec4(5.0f), 1.0f);
            subpanel->color(glm::vec4(0.0f));
            subpanel->orientation(Orientation::horizontal);

            InputBindBox* bindbox = new InputBindBox(entry.second);
            subpanel->add(bindbox);
            Label* label = new Label(langs::get(util::str2wstr_utf8(bindname)));
            label->margin(glm::vec4(6.0f));
            subpanel->add(label);
            scrollPanel->add(subpanel);
        }

        panel->add(scrollPanel);
        panel->add(guiutil::backButton(menu));
    }
}

void create_settings_panel(Engine* engine, PagesControl* menu) {
    auto panel = create_page(engine, "settings", 400, 0.0f, 1);

    {
        panel->add((new Label(L""))->textSupplier([=]() {
            return langs::get(L"Load Distance", L"settings") + L": " + std::to_wstring(engine->getSettings().chunks.loadDistance) + L" chunks";
        }));

        TrackBar* trackbar = new TrackBar(3, 66, 10, 1, 3);
        trackbar->supplier([=]() {
            return engine->getSettings().chunks.loadDistance;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().chunks.loadDistance = value;
        });
        panel->add(trackbar);
    }

    {
        panel->add((new Label(L""))->textSupplier([=]() {
            return langs::get(L"Load Speed", L"settings") + L": " + std::to_wstring(engine->getSettings().chunks.loadSpeed);
        }));

        TrackBar* trackbar = new TrackBar(1, 32, 10, 1, 1);
        trackbar->supplier([=]() {
            return engine->getSettings().chunks.loadSpeed;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().chunks.loadSpeed = value;
        });
        panel->add(trackbar);
    }

    {
        panel->add((new Label(L""))->textSupplier([=]() {
            std::wstringstream ss;
            ss << std::fixed << std::setprecision(1);
            ss << engine->getSettings().graphics.fogCurve;
            return langs::get(L"Fog Curve", L"settings") + L": " + ss.str();
        }));

        TrackBar* trackbar = new TrackBar(1.0, 6.0, 1.0, 0.1, 2);
        trackbar->supplier([=]() {
            return engine->getSettings().graphics.fogCurve;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().graphics.fogCurve = value;
        });
        panel->add(trackbar);
    }

    {
        panel->add((new Label(L""))->textSupplier([=]() {
            int fov = (int)engine->getSettings().camera.fov;
            return langs::get(L"FOV", L"settings") + L": " + std::to_wstring(fov) + L"°";
        }));

        TrackBar* trackbar = new TrackBar(30.0, 120.0, 90, 1, 4);
        trackbar->supplier([=]() {
            return engine->getSettings().camera.fov;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().camera.fov = value;
        });
        panel->add(trackbar);
    }

    {
        Panel* checkpanel = new Panel(glm::vec2(400, 32), glm::vec4(5.0f), 1.0f);
        checkpanel->color(glm::vec4(0.0f));
        checkpanel->orientation(Orientation::horizontal);

        CheckBox* checkbox = new CheckBox();
        checkbox->margin(glm::vec4(0.0f, 0.0f, 5.0f, 0.0f));
        checkbox->supplier([=]() {
            return engine->getSettings().display.swapInterval != 0;
        });
        checkbox->consumer([=](bool checked) {
            engine->getSettings().display.swapInterval = checked;
        });
        checkpanel->add(checkbox);
        checkpanel->add(new Label(langs::get(L"V-Sync", L"settings")));

        panel->add(checkpanel);
    }

    {
        Panel* checkpanel = new Panel(glm::vec2(400, 32), glm::vec4(5.0f), 1.0f);
        checkpanel->color(glm::vec4(0.0f));
        checkpanel->orientation(Orientation::horizontal);

        CheckBox* checkbox = new CheckBox();
        checkbox->margin(glm::vec4(0.0f, 0.0f, 5.0f, 0.0f));
        checkbox->supplier([=]() {
            return engine->getSettings().graphics.backlight != 0;
        });
        checkbox->consumer([=](bool checked) {
            engine->getSettings().graphics.backlight = checked;
        });
        checkpanel->add(checkbox);
        checkpanel->add(new Label(langs::get(L"Backlight", L"settings")));

        panel->add(checkpanel);
    }

    {
        std::string langName = langs::locales_info.at(langs::current->getId()).name;
        panel->add(guiutil::gotoButton(
            langs::get(L"Language", L"settings")+L": "+
            util::str2wstr_utf8(langName), 
            "languages", menu));
    }

    panel->add(guiutil::gotoButton(langs::get(L"Controls", L"menu"), "controls", menu));
    panel->add(guiutil::backButton(menu));
}

void create_pause_panel(Engine* engine, PagesControl* menu) {
    auto panel = create_page(engine, "pause", 400, 0.0f, 1);

    panel->add(create_button(L"Continue", glm::vec4(10.0f), glm::vec4(1), [=](GUI*){
        menu->reset();
    }));
    panel->add(guiutil::gotoButton(L"Settings", "settings", menu));

    panel->add(create_button(L"Save and Quit to Menu", glm::vec4(10.f), glm::vec4(1), [=](GUI*){
        engine->setScreen(std::make_shared<MenuScreen>(engine));
    }));
}

void create_languages_panel(Engine* engine, PagesControl* menu) {
    auto panel = create_page(engine, "languages", 400, 0.5f, 1);
    panel->scrollable(true);

    std::vector<std::string> locales;
    for (auto& entry : langs::locales_info) {
        locales.push_back(entry.first);
    }
    std::sort(locales.begin(), locales.end());
    for (std::string& name : locales) {
        auto& locale = langs::locales_info.at(name);
        std::string& fullName = locale.name;

        Button* button = new Button(util::str2wstr_utf8(fullName), glm::vec4(10.f));
        button->listenAction([=](GUI*) {
            auto resdir = engine->getPaths()->getResources();
            engine->setLanguage(name);
            menu->back();
        });
        panel->add(button);
    }
    panel->add(guiutil::backButton(menu));
}

void menus::create_menus(Engine* engine, PagesControl* menu) {
    create_new_world_panel(engine, menu);
    create_settings_panel(engine, menu);
    create_controls_panel(engine, menu);
    create_pause_panel(engine, menu);
    create_languages_panel(engine, menu);
    create_content_panel(engine, menu);
    create_main_menu_panel(engine, menu);
}

void menus::refresh_menus(Engine* engine, PagesControl* menu) {
    create_main_menu_panel(engine, menu);
}
