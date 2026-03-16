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
#include "coders/png.h"
#include "files/engine_paths.h"
#include "frontend/screens.h"
#include "content/content.h"
#include "frontend/locale/langs.h"
#include "util/platform.h"
#include "frontend/menu/menu.h"
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
#include "graphics/ui/elements/containers.h"

// Реализация конструктора
Engine::Engine(EngineSettings& settings, EnginePaths* paths) : settings(settings), paths(paths), settingsHandler(settings) {
    // Инициализация окна GLFW
    if (!Window::initialize(settings.display)) {
        LOG_CRITICAL("Failed to load Window");
        Window::terminate();
        throw initialize_error("Failed to load Window");
    }

    audio::initialize(settings.audio.enabled);
    audio::create_channel("regular");
    audio::create_channel("music");
    audio::create_channel("ambient");
    audio::create_channel("ui");

    settings.audio.volumeMaster.observe([=](auto value) {
        audio::get_channel("master")->setVolume(value * value);
    });
    settings.audio.volumeRegular.observe([=](auto value) {
        audio::get_channel("regular")->setVolume(value * value);
    });
    settings.audio.volumeUI.observe([=](auto value) {
        audio::get_channel("ui")->setVolume(value * value);
    });
    settings.audio.volumeAmbient.observe([=](auto value) {
        audio::get_channel("ambient")->setVolume(value * value);
    });
    settings.audio.volumeMusic.observe([=](auto value) {
        audio::get_channel("music")->setVolume(value * value);
    });

    auto resdir = paths->getResources();

    LOG_INFO("Loading assets");
    std::vector<std::filesystem::path> roots {resdir};
    resPaths = std::make_unique<ResPaths>(resdir, roots);
    assets = std::make_unique<Assets>();
	AssetsLoader loader(assets.get(), resPaths.get());
	AssetsLoader::addDefaults(loader, nullptr);

    ShaderProgram::preprocessor->setPaths(resPaths.get());

	while (loader.hasNext()) {
		if (!loader.loadNext()) {
			assets.reset();
            scripting::close();
			Window::terminate();
            LOG_ERROR("Could not to load assets");
			throw initialize_error("Could not to load assets");
		}
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
    if (screen) screen->onEngineShutdown();
    screen.reset();
    content.reset();
    assets.reset();
    audio::close();
    scripting::close();
    Window::terminate();
    LOG_INFO("Engine has finished successfuly");
    Logger::getInstance().flush();
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
    if (Events::justPressed(keycode::F2)) {
        std::unique_ptr<ImageData> image(Window::takeScreenshot());
		image->flipY();
		std::filesystem::path filename = paths->getScreenshotFile("png");
		png::writeImage(filename.string(), image.get());
    }

    if (Events::justPressed(keycode::F11)) Window::toggleFullscreen();
}

void Engine::onAssetsLoaded() {
    assets->store(new UIDocument(
        BUILTIN_CONTENT_NAMESPACE + ":root", 
        uidocscript {}, 
        std::dynamic_pointer_cast<gui::UINode>(gui->getContainer()), 
        nullptr
    ), BUILTIN_CONTENT_NAMESPACE + ":root");
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
        Window::swapInterval(Window::isIconified() ? 1 : settings.display.swapInterval);

        processPostRunnables();

        Window::swapBuffers(); // Показать отрендеренный кадр
        Events::pollEvents(); // Обработка событий ОС и ввода
    }
}

inline const std::string checkPacks(
    const std::unordered_set<std::string>& packs, 
    const std::vector<DependencyPack>& dependencies) 
{
    for (const auto& dependency : dependencies) { 
        if (packs.find(dependency.id) == packs.end()) {
            return dependency.id;
        }
    }
    return "";
}

void Engine::loadContent() {
    LOG_INFO("Loading content");
    auto resdir = paths->getResources();
    ContentBuilder contentBuilder;
    CoreContent::setup(&contentBuilder);
    paths->setContentPacks(&contentPacks);

    std::vector<std::filesystem::path> resRoots;
    std::vector<ContentPack> srcPacks = contentPacks;
    contentPacks.clear();

    std::string missingDependency;
	std::unordered_set<std::string> loadedPacks, existingPacks;
	for (const auto& item : srcPacks) {
        existingPacks.insert(item.id);
    }

	while (existingPacks.size() > loadedPacks.size()) {
		for (auto& pack : srcPacks) {
			if (loadedPacks.find(pack.id) != loadedPacks.end()) continue;
			missingDependency = checkPacks(existingPacks, pack.dependencies);
			if (!missingDependency.empty()) {
                LOG_ERROR("Missing dependency '{}'", missingDependency);
                throw contentpack_error(pack.id, pack.folder, "missing dependency '" + missingDependency + "'");
            }
			if (pack.dependencies.empty() || checkPacks(loadedPacks, pack.dependencies).empty()) {
				loadedPacks.insert(pack.id);
				resRoots.push_back(pack.folder);
				contentPacks.push_back(pack);
				ContentLoader loader(&pack);
				loader.load(contentBuilder);
			}
		}
    }
    content.reset(contentBuilder.build());
    resPaths = std::make_unique<ResPaths>(resdir, resRoots);

    langs::setup(resdir, langs::current->getId(), contentPacks);

	LOG_INFO("Loading content Assets");
    auto new_assets = std::make_unique<Assets>();
    ShaderProgram::preprocessor->setPaths(resPaths.get());
	AssetsLoader loader(new_assets.get(), resPaths.get());
	AssetsLoader::addDefaults(loader, content.get());
	while (loader.hasNext()) {
		if (!loader.loadNext()) {
			new_assets.reset();
            LOG_ERROR("Could not to initialize content assets");
			throw initialize_error("Could not to initialize content assets");
		}
	}
    assets->extend(*new_assets);
    LOG_INFO("Content Assets loaded successfully");
    LOG_INFO("Content loaded sucessfully");
    Logger::getInstance().flush();
}

void Engine::loadWorldContent(const std::filesystem::path& folder) {
    contentPacks.clear();
    auto packNames = ContentPack::worldPacksList(folder);
    ContentPack::readPacks(paths, contentPacks, packNames, folder);
    paths->setWorldFolder(folder);
    loadContent();
}

void Engine::loadAllPacks() {
	auto resdir = paths->getResources();
	contentPacks.clear();
	ContentPack::scan(paths, contentPacks);
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
    if (auto levelPtr = std::dynamic_pointer_cast<LevelScreen>(screen)) {
        menus::create_pause_panel(this, levelPtr->getLevelController());
    }
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
