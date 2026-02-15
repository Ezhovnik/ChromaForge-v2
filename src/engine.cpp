#include "engine.h"

#include <vector>
#include <ctime>
#include <exception>
#include <memory>
#include <assert.h>
#include <filesystem>

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
#include "assets/Assets.h"
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

// Реализация конструктора
Engine::Engine(EngineSettings& settings, EnginePaths* paths, Content* content) : settings(settings), content(content), paths(paths){
    // Инициализация окна GLFW
    if (!Window::initialize(settings.display)) {
        LOG_CRITICAL("Failed to load Window");
        Window::terminate();
        throw initialize_error("Failed to load Window");
    }

    ShaderProgram::preprocessor->setLibFolder(paths->getResources()/std::filesystem::path("shaders/lib"));

    // Загрузка ассетов
    assets = new Assets();
    LOG_INFO("Loading Assets");
    AssetsLoader loader(assets, paths->getResources());
    AssetsLoader::createDefaults(loader);
    AssetsLoader::addDefaults(loader);
    while (loader.hasNext()) {
        if (!loader.loadNext()) {
            delete assets;
            Window::terminate();
            LOG_CRITICAL("Could not to initialize assets");
            throw std::runtime_error("Could not to initialize assets");
        }
    }
    LOG_INFO("Assets loaded successfully");

    gui = new gui::GUI();

    LOG_INFO("Initialization is finished");
    Logger::getInstance().flush();
}

// Реализация деструктора
Engine::~Engine() {
    screen = nullptr;
    delete gui;

    LOG_INFO("Shutting down");
    delete assets;
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
    setScreen(std::shared_ptr<Screen>(new MenuScreen(this)));
    LOG_INFO("The menu screen has loaded successfully");

    LOG_INFO("Preparing systems");
    Batch2D batch(1024);
    lastTime = Window::time();
    Window::swapInterval(settings.display.swapInterval); // Включаем VSync (синхронизация с частотой обновления экрана)
    LOG_INFO("Systems have been prepared");

    Logger::getInstance().flush();

    while (!Window::isShouldClose()) {
        assert(screen != nullptr);

        updateTimers(); // Обновляем время и deltaTime
        updateHotkeys(); // Обрабатываем нажатия клавиш

        gui->activate(deltaTime);

        screen->update(deltaTime);
        screen->draw(deltaTime);

        gui->draw(&batch, assets);

        Window::swapInterval(settings.display.swapInterval);
        Window::swapBuffers(); // Показать отрендеренный кадр
        Events::pollEvents(); // Обработка событий ОС и ввода
    }
}

EnginePaths* Engine::getPaths() {
	return paths;
}

gui::GUI* Engine::getGUI() {
	return gui;
}

EngineSettings& Engine::getSettings() {
	return settings;
}

Assets* Engine::getAssets() {
	return assets;
}

const Content* Engine::getContent() const {
    return content;
}

void Engine::setScreen(std::shared_ptr<Screen> screen) {
	this->screen = screen;
}
