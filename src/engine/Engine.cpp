#include <engine/Engine.h>

#include <vector>
#include <memory>
#include <assert.h>
#include <filesystem>
#include <unordered_set>
#include <utility>

#define GLEW_STATIC

#include <window/Window.h>
#include <window/Events.h>
#include <window/input.h>
#include <assets/AssetsLoader.h>
#include <core_content_defs.h>
#include <debug/Logger.h>
#include <graphics/ui/GUI.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Viewport.h>
#include <graphics/core/ImageData.h>
#include <coders/GLSLExtension.h>
#include <coders/imageio.h>
#include <io/engine_paths.h>
#include <frontend/screens/Screen.h>
#include <frontend/screens/MenuScreen.h>
#include <content/content.h>
#include <frontend/locale/langs.h>
#include <util/platform.h>
#include <frontend/menu.h>
#include <content/Content.h>
#include <content/ContentLoader.h>
#include <logic/scripting/scripting.h>
#include <graphics/core/DrawContext.h>
#include <audio/audio.h>
#include <constants.h>
#include <frontend/UIDocument.h>
#include <graphics/ui/elements/UINode.h>
#include <content/PacksManager.h>
#include <util/listutil.h>
#include <logic/EngineController.h>
#include <io/settings_io.h>
#include <coders/toml.h>
#include <io/io.h>
#include <input_bindings.h>
#include <logic/CommandsInterpreter.h>
#include <content/ContentBuilder.h>
#include <objects/rigging.h>
#include <coders/commons.h>
#include <graphics/render/ModelsGenerator.h>
#include <network/Network.h>
#include <engine/Mainloop.h>
#include <engine/ServerMainloop.h>
#include <frontend/screens/LevelScreen.h>
#include <world/Level.h>
#include <logic/scripting/scripting_hud.h>

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

static std::unique_ptr<Engine> engine;

Engine& Engine::getInstance() {
    if (!engine) {
        engine = std::make_unique<Engine>();
    }
    return *engine;
}

void Engine::initialize(CoreParameters coreParameters) {
    params = std::move(coreParameters);
    settingsHandler = std::make_unique<SettingsHandler>(settings);
    interpreter = std::make_unique<cmd::CommandsInterpreter>();
    network = network::Network::create(settings.network);

    LOG_INFO("ChromaForge engine version: {}", ENGINE_VERSION_STRING);

    if (params.headless) {
        LOG_INFO("Headless mode is enabled");
    }
    paths.setResourcesFolder(params.resFolder);
    paths.setUserFilesFolder(params.userFolder);
    paths.prepare();
    if (!params.scriptFile.empty()) paths.setScriptFolder(params.scriptFile.parent_path());
    loadSettings();

    auto resdir = paths.getResourcesFolder();

    controller = std::make_unique<EngineController>(*this);

    // Инициализация окна GLFW
    if (!params.headless) {
        if (!Window::initialize(&this->settings.display)) {
            LOG_CRITICAL("Failed to load Window");
            Window::terminate();
            throw initialize_error("Failed to load Window");
        }
        time.set(Window::time());
        if (auto icon = load_icon()) {
            icon->flipY();
            if (icon->getFormat() != ImageFormat::rgba8888) icon.reset(toRGBA(icon.get()));
            Window::setIcon(icon.get());
        }
        loadControls();

        gui = std::make_unique<gui::GUI>();
        if (ENGINE_DEBUG_BUILD) {
            menus::create_version_label(*this);
        }
    }

    audio::initialize(!params.headless, settings.audio);

    LOG_INFO("Initialization of the scripting system");
    scripting::initialize(this);
    LOG_INFO("Scripting system initialization has been successfully finished");

    bool langNotSet = settings.ui.language.get() == "auto";
    if (langNotSet) settings.ui.language.set(platform::detect_locale());
    if (!isHeadless()) gui->setPageLoader(scripting::create_page_loader());
    keepAlive(settings.ui.language.observe([this](auto lang) {
        setLanguage(lang);
    }, true));

    basePacks = io::read_list("res:config/builtins.list");

    LOG_INFO("Initialization is finished");
    Logger::getInstance().flush();
}

// Реализация деструктора
Engine::~Engine() {
    LOG_INFO("Shutting down");
    saveSettings();
    if (screen) {
        screen->onEngineShutdown();
        screen.reset();
    }
    content.reset();
    assets.reset();
    interpreter.reset();
    if (gui) {
        gui.reset();
        LOG_INFO("GUI finished");
    }
    audio::close();
    network.reset();
    clearKeepedObjects();
    scripting::close();
    if (!params.headless) {
        Window::terminate();
        LOG_INFO("Window closed");
    }
    LOG_INFO("Engine has finished successfuly");
    Logger::getInstance().flush();
}

PacksManager Engine::createPacksManager(const io::path& worldFolder) {
    PacksManager manager;
    manager.setSources({
        {"world:content", worldFolder.empty() ? worldFolder : worldFolder / "content"},
        {"user:content", "user:content"},
        {"res:content", "res:content"}
    });
    return manager;
}

void Engine::setLevelConsumer(OnWorldOpen levelConsumer) {
    this->levelConsumer = std::move(levelConsumer);
}

void Engine::loadAssets() {
    LOG_INFO("Loading assets");
    ShaderProgram::preprocessor->setPaths(resPaths.get());

    auto new_assets = std::make_unique<Assets>();
    AssetsLoader loader(new_assets.get(), resPaths.get());
    AssetsLoader::addDefaults(loader, content.get());
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
        for (auto& [name, def] : content->blocks.getDefs()) {
            if (def->model == BlockModel::Custom) {
                if (def->modelName.empty()) {
                    assets->store(
                        std::make_unique<model::Model>(
                            ModelsGenerator::loadCustomBlockModel(
                                def->customModelRaw, *assets, !def->shadeless
                            )
                        ),
                        name + ".model"
                    );
                    def->modelName = def->name + ".model";
                }
            }
        }

        for (auto& [name, def] : content->items.getDefs()) {
            assets->store(
                std::make_unique<model::Model>(
                    ModelsGenerator::generate(*def, *content, *assets)
                ),
                name + ".model"
            );
        }
    }

    LOG_INFO("Assets loaded successfully");
}

// Обработка горячих клавиш
void Engine::updateHotkeys() {
    if (Events::justPressed(keycode::F2)) saveScreenshot();
    if (Events::justPressed(keycode::F8)) gui->toggleDebug();
    if (Events::justPressed(keycode::F11)) settings.display.fullscreen.toggle();
}

void Engine::saveScreenshot() {
    auto image = Window::takeScreenshot();
    image->flipY();
    io::path filename = paths.getNewScreenshotFile("png");
    imageio::write(filename.string(), image.get());
    LOG_INFO("Save screenshot as '{}'", filename.string());
}

void Engine::onAssetsLoaded() {
    assets->setup();
    gui->onAssetsLoad(assets.get());
}

void Engine::renderFrame() {
    screen->draw(time.getDeltaTime());

    Viewport viewport(Window::width, Window::height);
    DrawContext ctx(nullptr, viewport, nullptr);
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
    gui->activate(delta, Viewport(Window::width, Window::height));
    screen->update(delta);
    gui->postActivate();
}

void Engine::nextFrame() {
    Window::setFramerate(
        Window::isIconified() && settings.display.limitFpsIconified.get()
            ? 20
            : settings.display.framerate.get()
    );
    Window::swapBuffers();
    Events::pollEvents();
}

static void load_configs(const io::path& root) {
    auto configFolder = root / io::path("config");
    auto bindsFile = configFolder / io::path("bindings.toml");
    if (io::is_regular_file(bindsFile)) {
        Events::loadBindings(
            bindsFile.string(),
            io::read_string(bindsFile),
            BindType::Bind
        );
    }
}

void Engine::loadContent() {
    scripting::cleanup();

    LOG_INFO("Loading content");

    std::vector<std::string> names;
    for (auto& pack : contentPacks) {
        names.push_back(pack.id);
    }

    ContentBuilder contentBuilder;
    CoreContent::setup(contentBuilder);
    paths.setContentPacks(&contentPacks);
    PacksManager manager = createPacksManager(paths.getCurrentWorldFolder());
    manager.scan();
    names = manager.assemble(names);
    contentPacks = manager.getAll(names);

    auto builtinPack = ContentPack::createBuiltin(paths);

    std::vector<PathsRoot> resRoots {
        {BUILTIN_CONTENT_NAMESPACE, builtinPack.folder}
    };
    for (auto& pack : contentPacks) {
        resRoots.push_back({pack.id, pack.folder});
    }
    resPaths = std::make_unique<ResPaths>("res:", resRoots);
    {
        ContentLoader(&builtinPack, contentBuilder, *resPaths).load();
        load_configs(builtinPack.folder);
    }
    for (auto& pack : contentPacks) {
        ContentLoader(&pack, contentBuilder, *resPaths).load();
        load_configs(pack.folder);
    }

    content = contentBuilder.build();
    interpreter->reset();
    scripting::on_content_load(content.get());

    ContentLoader::loadScripts(*content);

    std::string locale = langs::current ? langs::current->getId() : langs::FALLBACK_DEFAULT;
    setLanguage(locale);

    if (!isHeadless()) {
        loadAssets();
        onAssetsLoaded();
    }

    LOG_INFO("Content loaded sucessfully");
    Logger::getInstance().flush();
}

void Engine::resetContent() {
    scripting::cleanup();

    std::vector<PathsRoot> resRoots;
    {
        auto pack = ContentPack::createBuiltin(paths);
        resRoots.push_back({BUILTIN_CONTENT_NAMESPACE, pack.folder});
        load_configs(pack.folder);
    }
    auto manager = createPacksManager(io::path());
    manager.scan();
    for (const auto& pack : manager.getAll(basePacks)) {
        resRoots.push_back({pack.id, pack.folder});
    }
    resPaths = std::make_unique<ResPaths>("res:", resRoots);
    contentPacks.clear();
    content.reset();

    std::string locale = langs::current ? langs::current->getId() : langs::FALLBACK_DEFAULT;
    setLanguage(locale);
    if (!isHeadless()) {
        loadAssets();
        onAssetsLoaded();
    }

    contentPacks = manager.getAll(basePacks);
}

void Engine::loadWorldContent(const io::path& folder) {
    contentPacks.clear();
    auto packNames = ContentPack::worldPacksList(folder);
    PacksManager manager;
    manager.setSources({
        {"world:content", folder.empty() ? folder : folder / "content"},
        {"user:content", "user:content"},
        {"res:content", "res:content"}
    });
    manager.scan();
    contentPacks = manager.getAll(manager.assemble(packNames));
    paths.setCurrentWorldFolder(folder);
    loadContent();
    loadControls();
}

void Engine::loadAllPacks() {
	PacksManager manager = createPacksManager(paths.getCurrentWorldFolder());
    manager.scan();
    auto allnames = manager.getAllNames();
    contentPacks = manager.getAll(manager.assemble(allnames));
}

EnginePaths& Engine::getPaths() {
	return paths;
}

ResPaths* Engine::getResPaths() {
    return resPaths.get();
}

gui::GUI* Engine::getGUI() {
	return gui.get();
}

EngineSettings& Engine::getSettings() {
	return settings;
}

Assets* Engine::getAssets() {
	return assets.get();
}

const Content* Engine::getContent() const {
    return content.get();
}

Content* Engine::getWriteableContent() {
    return content.get();
}

std::vector<ContentPack>& Engine::getContentPacks() {
    return contentPacks;
}

std::vector<ContentPack> Engine::getAllContentPacks() {
    auto packs = getContentPacks();
    packs.insert(packs.begin(), ContentPack::createBuiltin(paths));
    return packs;
}

EngineController* Engine::getController() {
    return controller.get();
}

void Engine::setScreen(std::shared_ptr<Screen> screen) {
    audio::reset_channel(audio::get_channel_index("regular"));
    audio::reset_channel(audio::get_channel_index("ambient"));
	this->screen = std::move(screen);
}

std::shared_ptr<Screen> Engine::getScreen() {
    return screen;
}

void Engine::setLanguage(std::string locale) {
	langs::setup("res:", std::move(locale), contentPacks);
}

SettingsHandler& Engine::getSettingsHandler() {
    return *settingsHandler;
}

std::vector<std::string>& Engine::getBasePacks() {
    return basePacks;
}

void Engine::saveSettings() {
    LOG_INFO("Writing the settings to a file");
    io::write_string(paths.getSettingsFile(), toml::stringify(*settingsHandler));
    LOG_INFO("The settings were successfully written to the file");

    if (!params.headless) {
        LOG_INFO("Writing the controls to a file");
        io::write_string(paths.getControlsFile(), Events::writeBindings());
        LOG_INFO("The controls were successfully written to the file");
    }
}

void Engine::loadSettings() {
    io::path settings_file = paths.getSettingsFile();
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
    io::path controls_file = paths.getControlsFile();
    if (io::is_regular_file(controls_file)) {
        LOG_INFO("Reading the controls file");
        std::string text = io::read_string(controls_file);
        Events::loadBindings(
            controls_file.string(),
            text,
            BindType::Bind
        );
        LOG_INFO("The controls file has been successfully read");
    }
}

cmd::CommandsInterpreter* Engine::getCommandsInterpreter() {
    return interpreter.get();
}

network::Network& Engine::getNetwork() {
    return *network;
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
        Window::setShouldClose(true);
    }
}

bool Engine::isQuitSignal() const {
    return quitSignal;
}

void Engine::terminate() {
    engine.reset();
}
