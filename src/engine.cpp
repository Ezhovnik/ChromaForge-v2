#include "engine.h"

#include <vector>
#include <ctime>
#include <exception>
#include <memory>
#include <assert.h>
#include <filesystem>
#include <iostream>

// GLM – библиотека для работы с матрицами и векторами в OpenGL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLEW_STATIC

// Пользовательские заголовочные файлы
#include "window/Window.h"
#include "window/Events.h"
#include "window/Camera.h"
#include "window/input.h"
#include "assets/AssetsLoader.h"
#include "definitions.h"
#include "logger/Logger.h"
#include "settings.h"
#include "frontend/gui/GUI.h"
#include "graphics/Batch2D.h"
#include "graphics/ImageData.h"
#include "graphics/ShaderProgram.h"
#include "coders/GLSLExtension.h"
#include "coders/png.h"
#include "files/engine_paths.h"
#include "frontend/screens.h"
#include "content/content.h"
#include "frontend/locale/langs.h"
#include "util/platform.h"
#include "frontend/menu.h"
#include "content/Content.h"
#include "content/ContentLoader.h"
#include "logic/scripting/scripting.h"

// Реализация конструктора
Engine::Engine(EngineSettings& settings, EnginePaths* paths) : settings(settings), paths(paths){
    // Инициализация окна GLFW
    if (!Window::initialize(settings.display)) {
        LOG_CRITICAL("Failed to load Window");
        Window::terminate();
        throw initialize_error("Failed to load Window");
    }

    auto resdir = paths->getResources();
    LOG_INFO("Initialization of the scripting system");
    scripting::initialize(this);
    LOG_INFO("Scripting system initialization has been successfully finished");

    LOG_INFO("Loading assets");
    std::vector<std::filesystem::path> roots {resdir};
    resPaths.reset(new ResPaths(resdir, roots));
    assets.reset(new Assets());
	AssetsLoader loader(assets.get(), resPaths.get());
	AssetsLoader::createDefaults(loader);
	AssetsLoader::addDefaults(loader, true);

    ShaderProgram::preprocessor->setPaths(resPaths.get());

	while (loader.hasNext()) {
		if (!loader.loadNext()) {
			assets.reset();
			Window::terminate();
            LOG_ERROR("Could not to initialize assets");
            Logger::getInstance().flush();
			throw initialize_error("Could not to initialize assets");
		}
	}

    gui = std::make_unique<gui::GUI>();

    if (settings.ui.language == "auto") settings.ui.language = platform::detect_locale();
    setLanguage(settings.ui.language);

    LOG_INFO("Initialization is finished");
    Logger::getInstance().flush();
}

// Реализация деструктора
Engine::~Engine() {
    scripting::close();
    screen = nullptr;

    LOG_INFO("Shutting down");
    assets.reset();
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

        gui->activate(deltaTime);

        screen->update(deltaTime);
    
        if (!Window::isIconified()) {
            screen->draw(deltaTime);
            gui->draw(&batch, assets.get());
            Window::swapInterval(settings.display.swapInterval);
        } else {
            Window::swapInterval(1);
        }

        Window::swapBuffers(); // Показать отрендеренный кадр
        Events::pollEvents(); // Обработка событий ОС и ввода
    }
}

void Engine::loadContent() {
    LOG_INFO("Loading content");
    auto resdir = paths->getResources();
    ContentBuilder contentBuilder;
    setup_definitions(&contentBuilder);

    std::vector<std::filesystem::path> resRoots;
    for (auto& pack : contentPacks) {
        ContentLoader loader(&pack);
        loader.load(&contentBuilder);
        resRoots.push_back(pack.folder);
    }
    content.reset(contentBuilder.build());
    resPaths.reset(new ResPaths(resdir, resRoots));

    ShaderProgram::preprocessor->setPaths(resPaths.get());

	std::unique_ptr<Assets> new_assets(new Assets());
	LOG_INFO("Loading content Assets");
	AssetsLoader loader(new_assets.get(), resPaths.get());
	AssetsLoader::createDefaults(loader);
	AssetsLoader::addDefaults(loader, false);
	while (loader.hasNext()) {
		if (!loader.loadNext()) {
			new_assets.reset();
            LOG_ERROR("Could not to initialize content assets");
            Logger::getInstance().flush();
			throw initialize_error("Could not to initialize content assets");
		}
	}
    assets->extend(*new_assets.get());
    LOG_INFO("Content Assets loaded successfully");
    LOG_INFO("Content loaded sucessfully");
    Logger::getInstance().flush();
}

void Engine::loadWorldContent(const std::filesystem::path& folder) {
    contentPacks.clear();
    auto packNames = ContentPack::worldPacksList(folder);
    ContentPack::readPacks(paths, contentPacks, packNames, folder);
    loadContent();
}

void Engine::loadAllPacks() {
	auto resdir = paths->getResources();
	contentPacks.clear();
	ContentPack::scan(resdir/std::filesystem::path("content"), contentPacks);
}

EnginePaths* Engine::getPaths() {
	return paths;
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
	this->screen = screen;
}

void Engine::setLanguage(std::string locale) {
	settings.ui.language = locale;
	langs::setup(paths->getResources(), locale, contentPacks);
	menus::create_menus(this, gui->getMenu());
}
