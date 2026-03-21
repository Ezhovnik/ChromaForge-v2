#include "engine.h"

#include <vector>
#include <memory>
#include <assert.h>
#include <filesystem>
#include <unordered_set>

#define GLEW_STATIC

#include "window/Window.h"
#include "window/Events.h"
#include "window/input.h"
#include "assets/AssetsLoader.h"
#include "core_content_defs.h"
#include "debug/Logger.h"
#include "settings.h"
#include "graphics/ui/GUI.h"
#include "graphics/core/Batch2D.h"
#include "graphics/core/ShaderProgram.h"
#include "graphics/core/Viewport.h"
#include "graphics/core/ImageData.h"
#include "coders/GLSLExtension.h"
#include "coders/imageio.h"
#include "files/engine_paths.h"
#include "frontend/screens/Screen.h"
#include "frontend/screens/MenuScreen.h"
#include "content/content.h"
#include "frontend/locale/langs.h"
#include "util/platform.h"
#include "frontend/menu.h"
#include "content/Content.h"
#include "content/ContentLoader.h"
#include "logic/scripting/scripting.h"
#include "graphics/core/GfxContext.h"
#include "world/WorldGenerators.h"
#include "voxels/DefaultWorldGenerator.h"
#include "voxels/FlatWorldGenerator.h"
#include "audio/audio.h"
#include "constants.h"
#include "frontend/UIDocument.h"
#include "graphics/ui/elements/UINode.h"
#include "content/PacksManager.h"
#include "util/listutil.h"
#include "logic/EngineController.h"

inline void create_channel(Engine* engine, std::string name, NumberSetting& setting) {
    if (name != "master") audio::create_channel(name);

    engine->keepAlive(setting.observe([=](auto value) {
        audio::get_channel(name)->setVolume(value * value);
    }));
}

// Реализация конструктора
Engine::Engine(EngineSettings& settings, EnginePaths* paths) : settings(settings), paths(paths), settingsHandler(settings) {
    controller = std::make_unique<EngineController>(this);

    // Инициализация окна GLFW
    if (!Window::initialize(settings.display)) {
        LOG_CRITICAL("Failed to load Window");
        Window::terminate();
        throw initialize_error("Failed to load Window");
    }

    audio::initialize(settings.audio.enabled);
    create_channel(this, "master", settings.audio.volumeMaster);
    create_channel(this, "regular", settings.audio.volumeRegular);
    create_channel(this, "music", settings.audio.volumeMusic);
    create_channel(this, "ambient", settings.audio.volumeAmbient);
    create_channel(this, "ui", settings.audio.volumeUI);

    auto resdir = paths->getResources();

    LOG_INFO("Loading assets");
    std::vector<std::pair<std::string, std::filesystem::path>> roots {};
    resPaths = std::make_unique<ResPaths>(resdir, roots);
    try {
        loadAssets();
    } catch (std::runtime_error& err) {
        LOG_ERROR("Fatal error occured while loading assets");

        assets.reset();
        scripting::close();
        audio::close();
        Window::terminate();
        throw initialize_error(err.what());
    }

    gui = std::make_unique<gui::GUI>();

    if (settings.ui.language == "auto") settings.ui.language = platform::detect_locale();
    if (ENGINE_VERSION_INDEV) menus::create_version_label(this);
    setLanguage(settings.ui.language);

    addDefaultWorldGenerators();

    onAssetsLoaded();

    LOG_INFO("Initialization of the scripting system");
    scripting::initialize(this);
    LOG_INFO("Scripting system initialization has been successfully finished");

    LOG_INFO("Initialization is finished");
    Logger::getInstance().flush();
}

// Реализация деструктора
Engine::~Engine() {
    LOG_INFO("Shutting down");
    if (screen) {
        screen->onEngineShutdown();
        screen.reset();
    }
    content.reset();
    assets.reset();
    gui.reset();
    audio::close();
    scripting::close();
    Window::terminate();
    LOG_INFO("Engine has finished successfuly");
    Logger::getInstance().flush();
}

PacksManager Engine::createPacksManager(const std::filesystem::path& worldFolder) {
    PacksManager manager;
    manager.setSources({
        worldFolder/std::filesystem::path("content"),
        paths->getUserfiles()/std::filesystem::path("content"),
        paths->getResources()/std::filesystem::path("content")
    });
    return manager;
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
            if (!loader.loadNext()) {
                new_assets.reset();
                LOG_ERROR("Could not to load assets");
                throw std::runtime_error("Could not to load assets");
            }
        }
    }

    if (assets) {
        assets->extend(*new_assets);
    } else {
        assets.reset(new_assets.release());
    }
    LOG_INFO("Assets loaded successfully");
}

// Обновление таймеров
void Engine::updateTimers() {
    frame++;
    double currentTime = Window::time(); // Текущее время от GLFW
    deltaTime = currentTime - lastTime; // Расчёт времени между кадрами
    lastTime = currentTime; // Обновление lastTime для следующего кадра
}

// Обработка горячих клавиш
void Engine::updateHotkeys() {
    if (Events::justPressed(keycode::F2)) saveScreenshot();
    if (Events::justPressed(keycode::F11)) Window::toggleFullscreen();
}

void Engine::saveScreenshot() {
    std::unique_ptr<ImageData> image(Window::takeScreenshot());
    image->flipY();
    std::filesystem::path filename = paths->getScreenshotFile("png");
    imageio::write(filename.string(), image.get());
    LOG_INFO("Save screenshot as '{}'", filename.u8string());
}

void Engine::onAssetsLoaded() {
    gui->onAssetsLoad(assets.get());
}

void Engine::renderFrame(Batch2D& batch) {
    screen->draw(deltaTime);

    Viewport viewport(Window::width, Window::height);
    GfxContext ctx(nullptr, viewport, &batch);
    gui->draw(&ctx, assets.get());
}

void Engine::processPostRunnables() {
    std::lock_guard<std::recursive_mutex> lock(postRunnablesMutex);
    while (!postRunnables.empty()) {
        postRunnables.front()();
        postRunnables.pop();
    }
    scripting::process_post_runnables();
}

// Основной цикл приложения
void Engine::mainloop() {
    LOG_INFO("Loading the menu screen");
    setScreen(std::make_shared<MenuScreen>(this));
    LOG_INFO("The menu screen has loaded successfully");

    LOG_INFO("Preparing systems");
    Batch2D batch(1024);
    lastTime = Window::time();
    LOG_INFO("Systems have been prepared");

    Logger::getInstance().flush();

    while (!Window::isShouldClose()) {
        assert(screen != nullptr);

        updateTimers(); // Обновляем время и deltaTime
        updateHotkeys(); // Обрабатываем нажатия клавиш

        audio::update(deltaTime);

        gui->activate(deltaTime);

        screen->update(deltaTime);

        if (!Window::isIconified()) renderFrame(batch);
        Window::swapInterval(Window::isIconified() ? 1 : settings.display.vsync.get());

        processPostRunnables();

        Window::swapBuffers(); // Показать отрендеренный кадр
        Events::pollEvents(); // Обработка событий ОС и ввода
    }
}

void Engine::loadContent() {
    LOG_INFO("Loading content");
    auto resdir = paths->getResources();
    ContentBuilder contentBuilder;
    CoreContent::setup(&contentBuilder);
    paths->setContentPacks(&contentPacks);

    std::vector<std::string> names;
    for (auto& pack : contentPacks) {
        names.push_back(pack.id);
    }

    PacksManager manager = createPacksManager(paths->getWorldFolder());
    manager.scan();
    names = manager.assembly(names);
    contentPacks = manager.getAll(names);

    std::vector<std::pair<std::string, std::filesystem::path>> resRoots;
    for (auto& pack : contentPacks) {
        resRoots.push_back({pack.id, pack.folder});

        ContentLoader loader(&pack);
        loader.load(contentBuilder);
    }

    content.reset(contentBuilder.build());
    resPaths = std::make_unique<ResPaths>(resdir, resRoots);

    langs::setup(resdir, langs::current->getId(), contentPacks);

	loadAssets();
    onAssetsLoaded();
    LOG_INFO("Content loaded sucessfully");
    Logger::getInstance().flush();
}

void Engine::loadWorldContent(const std::filesystem::path& folder) {
    contentPacks.clear();
    auto packNames = ContentPack::worldPacksList(folder);
    PacksManager manager;
    manager.setSources({
        folder/std::filesystem::path("content"),
        paths->getUserfiles()/std::filesystem::path("content"),
        paths->getResources()/std::filesystem::path("content")
    });
    manager.scan();
    contentPacks = manager.getAll(manager.assembly(packNames));
    paths->setWorldFolder(folder);
    loadContent();
}

void Engine::loadAllPacks() {
	PacksManager manager;
    manager.setSources({
        paths->getWorldFolder()/std::filesystem::path("content"),
        paths->getUserfiles()/std::filesystem::path("content"),
        paths->getResources()/std::filesystem::path("content")
    });
    manager.scan();
    auto allnames = manager.getAllNames();
    contentPacks = manager.getAll(manager.assembly(allnames));
}

EnginePaths* Engine::getPaths() {
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

std::vector<ContentPack>& Engine::getContentPacks() {
    return contentPacks;
}

EngineController* Engine::getController() {
    return controller.get();
}

void Engine::setScreen(std::shared_ptr<Screen> screen) {
    audio::reset_channel(audio::get_channel_index("regular"));
    audio::reset_channel(audio::get_channel_index("ambient"));
	this->screen = screen;
}

std::shared_ptr<Screen> Engine::getScreen() {
    return screen;
}

double Engine::getDeltaTime() const {
    return deltaTime;
}

void Engine::setLanguage(std::string locale) {
	settings.ui.language = locale;
	langs::setup(paths->getResources(), locale, contentPacks);
	menus::create_menus(this);
}

void Engine::addDefaultWorldGenerators() {
	WorldGenerators::addGenerator<DefaultWorldGenerator>(BUILTIN_CONTENT_NAMESPACE + ":default");
	WorldGenerators::addGenerator<FlatWorldGenerator>(BUILTIN_CONTENT_NAMESPACE + ":flat");
}

void Engine::postRunnable(runnable callback) {
    postRunnables.push(callback);
}

SettingsHandler& Engine::getSettingsHandler() {
    return settingsHandler;
}
