#include <engine/Engine.h>

#include <vector>
#include <memory>
#include <assert.h>
#include <filesystem>
#include <unordered_set>

#define GLEW_STATIC

#include <window/Window.h>
#include <window/input.h>
#include <assets/AssetsLoader.h>
#include <core_content_defs.h>
#include <debug/Logger.h>
#include <graphics/ui/GUI.h>
#include <graphics/core/ShaderProgram.h>
#include <coders/GLSLExtension.h>
#include <engine/EnginePaths.h>
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
#include <devtools/DebuggingServer.h>
#include <graphics/ui/elements/Menu.h>
#include <engine/WindowControl.h>
#include <engine/AssetsManagement.h>
#include <devtools/stdin_cmd_reader.h>

static debug::Logger logger("engine");

Engine::Engine() = default;
Engine::~Engine() = default;

static std::unique_ptr<Engine> instance = nullptr;

Engine& Engine::getInstance() {
    if (!instance) {
        instance = std::make_unique<Engine>();
    }
    return *instance;
}

void Engine::onContentLoad() {
    editor->loadTools();
    langs::setup(langs::get_current(), paths->resPaths.collectRoots());

    if (isHeadless()) return;

    for (auto& pack : content->getAllContentPacks()) {
        auto configFolder = pack.folder / "config";
        auto bindsFile = configFolder / "bindings.toml";
        logger.info() << "Loading bindings: " << bindsFile.string();
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

void Engine::initializeClient() {
    assets = std::make_unique<AssetsManagement>(*this);
    windowControl = std::make_unique<WindowControl>(*this);
    auto [window, input] = windowControl->initialize();

    this->window = std::move(window);
    this->input = std::move(input);

    loadControls();

    gui = std::make_unique<gui::GUI>(*this);

    if (ENGINE_DEBUG_BUILD) {
        menus::create_version_label(*gui);
    }

    keepAlive(settings.display.windowMode.observe(
        [this](int value) {
            WindowMode mode = static_cast<WindowMode>(value);
            if (mode != this->window->getMode()) {
                this->window->setMode(mode);
            }
        },
        true
    ));
    keepAlive(settings.debug.doTraceShaders.observe(
        [](bool value) {
            ShaderProgram::preprocessor->setTraceOutput(value);
        },
        true
    ));
    keepAlive(this->input->addKeyCallback(Keycode::ESCAPE, [this]() {
        auto& menu = *gui->getMenu();
        if (menu.hasOpenPage() && menu.back()) {
            return true;
        }
        return false;
    }));
}

void Engine::initialize(CoreParameters coreParameters) {
    params = std::move(coreParameters);
    settingsHandler = std::make_unique<SettingsHandler>(settings);

    logger.info() << "ChromaForge engine version: " << ENGINE_VERSION_STRING;

    if (params.headless) {
        logger.info() << "Engine runs in headless mode";
    }
    if (params.projectFolder.empty()) {
        params.projectFolder = params.resFolder;
    }
    paths = std::make_unique<EnginePaths>(params);
    loadProject();
    paths->setupProject(*project);

    editor = std::make_unique<devtools::Editor>(*this);
    cmd = std::make_unique<cmd::CommandsInterpreter>();
    if (project->permissions.has(Permissions::NETWORK) || !params.debugServerString.empty()) {
        network = network::Network::create(settings.network);
    }

    if (!params.debugServerString.empty()) {
        try {
            debuggingServer = std::make_unique<devtools::DebuggingServer>(
                *this, params.debugServerString
            );
        } catch (const std::runtime_error& err) {
            throw initialize_error(
                "Debugging server error: " + std::string(err.what())
            );
        }
    }

    loadSettings();

    controller = std::make_unique<EngineController>(*this);

    if (!params.headless) initializeClient();

    audio::initialize(
        !params.headless,
        project->permissions.has(Permissions::RECORD_AUDIO),
        settings.audio
    );

    if (settings.ui.language.get() == "auto") {
        settings.ui.language.set(
            langs::locale_by_envlocale(platform::detect_locale())
        );
    }

    content = std::make_unique<ContentControl>(
        *project, *paths, input.get(), [this]() {onContentLoad();}
    );

    logger.info() << "Initialization of the scripting system";
    scripting::initialize(this);
    logger.info() << "Scripting system initialization has been successfully finished";

    if (!isHeadless()) gui->getMenu()->setPageLoader(scripting::create_page_loader());
    keepAlive(settings.ui.language.observe([this](auto lang) {
        langs::setup(lang, paths->resPaths.collectRoots());
    }, true));

    keepAlive(settings.audio.inputDevice.observe([](auto name) {
        audio::set_input_device(name == "auto" ? "" : name);
    }, true));

    project->loadProjectStartScript();
    if (!params.headless) {
        project->loadProjectClientScript();
    }

    if (params.stdinCommands) {
        cmd::start_stdin_cmd_reader(*this);
    }

    logger.info() << "Initialization is finished";
}

void Engine::close() {
    logger.info() << "Shutting down";
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
        logger.info() << "GUI finished";
    }
    audio::close();
    debuggingServer.reset();
    network.reset();
    clearKeepedObjects();
    project.reset();
    scripting::close();
    if (!params.headless) {
        window.reset();
        logger.info() << "Window closed";
    }
    logger.info() << "Engine has finished successfuly";
}

void Engine::setLevelConsumer(OnWorldOpen levelConsumer) {
    this->levelConsumer = std::move(levelConsumer);
}

void Engine::loadAssets() {
    assets->loadAssets(content->get());
}

// Обработка горячих клавиш
void Engine::updateHotkeys() {
    if (input->justPressed(Keycode::F2)) {
        windowControl->saveScreenshot();
    }
    if (input->isPressed(Keycode::LEFT_CONTROL) &&
        input->isPressed(Keycode::F3) &&
        input->justPressed(Keycode::U)
    ) {
        gui->toggleDebug();
    }
    if (input->justPressed(Keycode::F11)) {
        windowControl->toggleFullscreen();
    }
}

void Engine::renderFrame() {
    if (input->isCursorLocked() != (gui->getActiveFrame() == nullptr)) {
        input->toggleCursor();
    }
    screen->draw(time.getDeltaTime());

    DrawContext ctx(nullptr, *window, nullptr);
    gui->draw(ctx, *assets->getStorage());
}

void Engine::run() {
    if (params.headless) {
        ServerMainloop(*this).run();
    } else {
        Mainloop(*this).run();
    }
}

void Engine::postUpdate() {
    if (network) {
        network->update();
    }
    postRunnables.run();
    scripting::process_post_runnables();

    if (debuggingServer) {
        debuggingServer->update();
    }
}

void Engine::detachDebugger() {
    debuggingServer.reset();
}

void Engine::applicationSpark() {
    if (project->setupCoroutine && project->setupCoroutine->isActive()) {
        project->setupCoroutine->update();
    }
}

void Engine::updateFrontend() {
    double delta = time.getDeltaTime();
    assets->update();
    updateHotkeys();
    audio::update(delta);
    gui->activate(delta, window->getSize());
    screen->update(delta);
    gui->postActivate();
}

void Engine::nextFrame(bool waitForRefresh) {
    windowControl->nextFrame(waitForRefresh);
}

AssetsLoader& Engine::acquireBackgroundLoader() {
    return assets->acquireBackgroundLoader();
}

EnginePaths& Engine::getPaths() {
	return *paths;
}

ResPaths& Engine::getResPaths() {
    return paths->resPaths;
}

EngineSettings& Engine::getSettings() {
	return settings;
}

Assets* Engine::getAssets() {
	return assets->getStorage();
}

EngineController* Engine::getController() {
    return controller.get();
}

void Engine::loadProject() {
    io::path projectFile = "project:project.toml";
    project = std::make_unique<Project>();
    project->deserialize(io::read_object(projectFile));
    logger.info() << "Loaded project" << util::quote(project->name);
}

void Engine::setScreen(std::shared_ptr<Screen> screen) {
    if (project->clientScript && this->screen) {
        project->clientScript->onScreenChange(this->screen->getName(), false);
    }

    audio::reset_channel(audio::get_channel_index("regular"));
    audio::reset_channel(audio::get_channel_index("ambient"));
	this->screen = std::move(screen);

    if (this->screen) this->screen->onOpen();
    if (project->clientScript && this->screen) {
        project->clientScript->onScreenChange(this->screen->getName(), true);
        window->setShouldRefresh();
    }
}

std::shared_ptr<Screen> Engine::getScreen() {
    return screen;
}

SettingsHandler& Engine::getSettingsHandler() {
    return *settingsHandler;
}

void Engine::saveSettings() {
    logger.info() << "Writing the settings to a file";
    io::write_string(EnginePaths::SETTINGS_FILE, toml::stringify(*settingsHandler));
    logger.info() << "The settings were successfully written to the file";

    if (!params.headless && input) {
        logger.info() << "Writing the controls to a file";
        io::write_string(EnginePaths::CONTROLS_FILE, input->getBindings().write());
        logger.info() << "The controls were successfully written to the file";
    }
}

void Engine::loadSettings() {
    io::path settings_file = EnginePaths::SETTINGS_FILE;
    if (io::is_regular_file(settings_file)) {
        logger.info() << "Reading the settings file";
        std::string text = io::read_string(settings_file);
        try {
            toml::parse(*settingsHandler, settings_file.string(), text);
        } catch (const parsing_error& err) {
            logger.error() << err.errorLog();
            throw;
        }
        logger.info() << "The settings file has been successfully read";
    }
}

void Engine::loadControls() {
    io::path controls_file = EnginePaths::CONTROLS_FILE;
    if (io::is_regular_file(controls_file)) {
        logger.info() << "Reading the controls file";
        std::string text = io::read_string(controls_file);
        input->getBindings().read(
            toml::parse(controls_file.string(), text), BindType::Bind
        );
        logger.info() << "The controls file has been successfully read";
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
    logger.info() << "World open";
    levelConsumer(std::move(level), localPlayer);
}

void Engine::onWorldClosed() {
    logger.info() << "World closed";
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

void Engine::startPauseLoop() {
    assert (network != nullptr);

    bool initialCursorLocked = false;
    if (!isHeadless()) {
        initialCursorLocked = input->isCursorLocked();
        if (initialCursorLocked) {
            input->toggleCursor();
        }
    }
    while (!isQuitSignal() && debuggingServer) {
        network->update();
        if (debuggingServer->update()) {
            break;
        }
        if (isHeadless()) {
            platform::sleep(1.0 / params.sps * 1000);
        } else {
            nextFrame(false);
        }
    }
    if (initialCursorLocked) {
        input->toggleCursor();
    }
}

