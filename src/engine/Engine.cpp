#include <engine/Engine.h>

#include <vector>
#include <memory>
#include <assert.h>
#include <filesystem>
#include <unordered_set>
#include <utility>

#define GLEW_STATIC

#include <window/Window.h>
#include <window/input.h>
#include <assets/AssetsLoader.h>
#include <core_content_defs.h>
#include <debug/Logger.h>
#include <graphics/ui/GUI.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/ImageData.h>
#include <coders/GLSLExtension.h>
#include <coders/imageio.h>
#include <io/engine_paths.h>
#include <frontend/screens/Screen.h>
#include <frontend/screens/MenuScreen.h>
#include <frontend/locale.h>
#include <util/platform.h>
#include <frontend/menu.h>
#include <logic/scripting/scripting.h>
#include <graphics/core/DrawContext.h>
#include <audio/audio.h>
#include <constants.h>
#include <frontend/UIDocument.h>
#include <graphics/ui/elements/UINode.h>
#include <content/PacksManager.h>
#include <logic/EngineController.h>
#include <io/settings_io.h>
#include <coders/toml.h>
#include <io/io.h>
#include <input_bindings.h>
#include <logic/CommandsInterpreter.h>
#include <objects/rigging.h>
#include <coders/commons.h>
#include <graphics/render/ModelsGenerator.h>
#include <network/Network.h>
#include <engine/Mainloop.h>
#include <engine/ServerMainloop.h>
#include <frontend/screens/LevelScreen.h>
#include <world/Level.h>
#include <logic/scripting/scripting_hud.h>
#include <content/ContentControl.h>
#include <devtools/Editor.h>
#include <devtools/Project.h>

static std::unique_ptr<ImageData> load_icon() {
    try {
        auto file = "res:textures/misc/icon.png";
        if (io::exists(file)) {
            return imageio::read(file);
        }
    } catch (const std::exception& err) {
        LOG_ERROR("Could not load window icon: {}", err.what());
    }
    return nullptr;
}

Engine::Engine() = default;
Engine::~Engine() = default;

static std::unique_ptr<Engine> instance = nullptr;

Engine& Engine::getInstance() {
    if (!instance) {
        instance = std::make_unique<Engine>();
    }
    return *instance;
}

void Engine::initialize(CoreParameters coreParameters) {
    params = std::move(coreParameters);
    settingsHandler = std::make_unique<SettingsHandler>(settings);

    LOG_INFO("ChromaForge engine version: {}", ENGINE_VERSION_STRING);

    if (params.headless) {
        LOG_INFO("Headless mode is enabled");
    }
    if (params.projectFolder.empty()) {
        params.projectFolder = params.resFolder;
    }
    paths.setResourcesFolder(params.resFolder);
    paths.setUserFilesFolder(params.userFolder);
    paths.setProjectFolder(params.projectFolder);
    paths.prepare();
    loadProject();

    editor = std::make_unique<devtools::Editor>(*this);
    cmd = std::make_unique<cmd::CommandsInterpreter>();
    network = network::Network::create(settings.network);

    if (!params.scriptFile.empty()) paths.setScriptFolder(params.scriptFile.parent_path());
    loadSettings();

    controller = std::make_unique<EngineController>(*this);

    // Инициализация окна GLFW
    if (!params.headless) {
        std::string title = project->title;
        if (title.empty()) title = "ChromaForge v" + ENGINE_VERSION_STRING;
        if (ENGINE_DEBUG_BUILD) title += " [development build]";

        auto [window, input] = Window::initialize(&settings.display, title);
        if (!window || !input){
            LOG_CRITICAL("Failed to load Window");
            throw initialize_error("Failed to load Window");
        }
        window->setFramerate(settings.display.framerate.get());

        time.set(window->time());
        if (auto icon = load_icon()) {
            icon->flipY();
            if (icon->getFormat() != ImageFormat::rgba8888) icon.reset(toRGBA(icon.get()));
            window->setIcon(icon.get());
        }
        this->window = std::move(window);
        this->input = std::move(input);

        loadControls();

        gui = std::make_unique<gui::GUI>(*this);
        if (ENGINE_DEBUG_BUILD) {
            menus::create_version_label(*gui);
        }
        keepAlive(settings.display.fullscreen.observe(
            [this](bool value) {
                if (value != this->window->isFullscreen()) {
                    this->window->toggleFullscreen();
                }
            },
            true
        ));
    }

    audio::initialize(!params.headless, settings.audio);

    bool langNotSet = settings.ui.language.get() == "auto";
    if (langNotSet) {
        settings.ui.language.set(
            langs::locale_by_envlocale(platform::detect_locale())
        );
    }

    content = std::make_unique<ContentControl>(*project, paths, *input, [this]() {
        editor->loadTools();
        langs::setup(langs::get_current(), paths.resPaths.collectRoots());
        if (!isHeadless()) {
            for (auto& pack : content->getAllContentPacks()) {
                auto configFolder = pack.folder / "config";
                auto bindsFile = configFolder / "bindings.toml";
                if (io::is_regular_file(bindsFile)) {
                    input->getBindings().read(
                        toml::parse(
                            bindsFile.string(), io::read_string(bindsFile)
                        ),
                        BindType::Bind
                    );
                }
            }
            loadAssets();
        }
    });

    LOG_INFO("Initialization of the scripting system");
    scripting::initialize(this);
    LOG_INFO("Scripting system initialization has been successfully finished");

    if (!isHeadless()) gui->setPageLoader(scripting::create_page_loader());
    keepAlive(settings.ui.language.observe([this](auto lang) {
        langs::setup(lang, paths.resPaths.collectRoots());
    }, true));

    LOG_INFO("Initialization is finished");
    Logger::getInstance().flush();
}

void Engine::close() {
    LOG_INFO("Shutting down");
    saveSettings();
    if (screen) {
        screen->onEngineShutdown();
        screen.reset();
    }
    content.reset();
    assets.reset();
    cmd.reset();
    if (gui) {
        gui.reset();
        LOG_INFO("GUI finished");
    }
    audio::close();
    network.reset();
    clearKeepedObjects();
    scripting::close();
    if (!params.headless) {
        window.reset();
        LOG_INFO("Window closed");
    }
    LOG_INFO("Engine has finished successfuly");
    Logger::getInstance().flush();
}

void Engine::setLevelConsumer(OnWorldOpen levelConsumer) {
    this->levelConsumer = std::move(levelConsumer);
}

void Engine::loadAssets() {
    LOG_INFO("Loading assets");
    ShaderProgram::preprocessor->setPaths(&paths.resPaths);

    auto content = this->content->get();
    auto new_assets = std::make_unique<Assets>();
    AssetsLoader loader(*this, *new_assets, paths.resPaths);
    AssetsLoader::addDefaults(loader, content);
    bool threading = false;
    if (threading) {
        auto task = loader.startTask([=](){});
        task->waitForEnd();
    } else {
        while (loader.hasNext()) {
            loader.loadNext();
        }
    }

    assets = std::move(new_assets);

    if (content) {
        ModelsGenerator::prepare(*content, *assets);
    }
    assets->setup();
    gui->onAssetsLoad(assets.get());
    LOG_INFO("Assets loaded successfully");
}

// Обработка горячих клавиш
void Engine::updateHotkeys() {
    if (input->justPressed(Keycode::F2)) saveScreenshot();
    if (input->isPressed(Keycode::LEFT_CONTROL) && input->isPressed(Keycode::F3) && input->justPressed(Keycode::U)) gui->toggleDebug();
    if (input->justPressed(Keycode::F11)) settings.display.fullscreen.toggle();
}

void Engine::saveScreenshot() {
    auto image = window->takeScreenshot();
    image->flipY();
    io::path filename = paths.getNewScreenshotFile("png");
    imageio::write(filename.string(), image.get());
    LOG_INFO("Save screenshot as '{}'", filename.string());
}

void Engine::renderFrame() {
    screen->draw(time.getDeltaTime());

    DrawContext ctx(nullptr, *window, nullptr);
    gui->draw(ctx, *assets);
}

void Engine::run() {
    if (params.headless) {
        ServerMainloop(*this).run();
    } else {
        Mainloop(*this).run();
    }
}

void Engine::postUpdate() {
    network->update();
    postRunnables.run();
    scripting::process_post_runnables();
}

void Engine::updateFrontend() {
    double delta = time.getDeltaTime();
    updateHotkeys();
    audio::update(delta);
    gui->activate(delta, window->getSize());
    screen->update(delta);
    gui->postActivate();
}

void Engine::nextFrame() {
    window->setFramerate(
        window->isIconified() && settings.display.limitFpsIconified.get()
            ? 20
            : settings.display.framerate.get()
    );
    window->swapBuffers();
    input->pollEvents();
}

EnginePaths& Engine::getPaths() {
	return paths;
}

ResPaths& Engine::getResPaths() {
    return paths.resPaths;
}

EngineSettings& Engine::getSettings() {
	return settings;
}

Assets* Engine::getAssets() {
	return assets.get();
}

EngineController* Engine::getController() {
    return controller.get();
}

void Engine::loadProject() {
    io::path projectFile = "project:project.toml";
    project = std::make_unique<Project>();
    project->deserialize(io::read_object(projectFile));
    LOG_INFO("Loaded project {}", util::quote(project->name));
}

void Engine::setScreen(std::shared_ptr<Screen> screen) {
    audio::reset_channel(audio::get_channel_index("regular"));
    audio::reset_channel(audio::get_channel_index("ambient"));
	this->screen = std::move(screen);
}

std::shared_ptr<Screen> Engine::getScreen() {
    return screen;
}

SettingsHandler& Engine::getSettingsHandler() {
    return *settingsHandler;
}

void Engine::saveSettings() {
    LOG_INFO("Writing the settings to a file");
    io::write_string(EnginePaths::SETTINGS_FILE, toml::stringify(*settingsHandler));
    LOG_INFO("The settings were successfully written to the file");

    if (!params.headless) {
        LOG_INFO("Writing the controls to a file");
        io::write_string(EnginePaths::CONTROLS_FILE, input->getBindings().write());
        LOG_INFO("The controls were successfully written to the file");
    }
}

void Engine::loadSettings() {
    io::path settings_file = EnginePaths::SETTINGS_FILE;
    if (io::is_regular_file(settings_file)) {
        LOG_INFO("Reading the settings file");
        std::string text = io::read_string(settings_file);
        try {
            toml::parse(*settingsHandler, settings_file.string(), text);
        } catch (const parsing_error& err) {
            LOG_ERROR("{}", err.errorLog());
            throw;
        }
        LOG_INFO("The settings file has been successfully read");
    }
}

void Engine::loadControls() {
    io::path controls_file = EnginePaths::CONTROLS_FILE;
    if (io::is_regular_file(controls_file)) {
        LOG_INFO("Reading the controls file");
        std::string text = io::read_string(controls_file);
        input->getBindings().read(
            toml::parse(controls_file.string(), text), BindType::Bind
        );
        LOG_INFO("The controls file has been successfully read");
    }
}

const CoreParameters& Engine::getCoreParameters() const {
    return params;
}

bool Engine::isHeadless() const {
    return params.headless;
}

EngineTime& Engine::getTime() {
    return time;
}

void Engine::onWorldOpen(std::unique_ptr<Level> level, int64_t localPlayer) {
    LOG_INFO("World open");
    levelConsumer(std::move(level), localPlayer);
}

void Engine::onWorldClosed() {
    LOG_INFO("World closed");
    levelConsumer(nullptr, -1);
}

void Engine::quit() {
    quitSignal = true;
    if (!isHeadless()) {
        window->setShouldClose(true);
    }
}

bool Engine::isQuitSignal() const {
    return quitSignal;
}

void Engine::terminate() {
    instance->close();
    instance.reset();
}

ContentControl& Engine::getContentControl() {
    return *content;
}
