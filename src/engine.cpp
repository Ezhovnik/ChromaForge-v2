#include "engine.h"

#include <assert.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <exception>
#include <filesystem>

#include <glm/glm.hpp>

#define GLEW_STATIC

// Пользовательские заголовочные файлы
#include "settings.h"
#include "coders/json.h"
#include "graphics/ImageData.h"
#include "files/engine_files.h"
#include "coders/png.h"
#include "files/files.h"
#include "window/Events.h"
#include "window/Camera.h"
#include "window/Window.h"
#include "window/input.h"
#include "voxels/Chunk.h"
#include "voxels/Chunks.h"
#include "voxels/ChunksController.h"
#include "voxels/ChunksStorage.h"
#include "assets/Assets.h"
#include "assets/AssetsLoader.h"
#include "objects/Player.h"
#include "definitions.h"
#include "frontend/world_render.h"
#include "world/Level.h"
#include "world/World.h"
#include "frontend/hud.h"
#include "frontend/gui/GUI.h"
#include "graphics/Batch2D.h"
#include "frontend/screens.h"
#include "logger/Logger.h"

// Точка спавна игрока и начальная скорость
inline constexpr glm::vec3 SPAWNPOINT = {0, 128, 0}; // Точка, где игрок появляется в мире
inline constexpr float DEFAULT_PLAYER_SPEED = 5.0f; // Начальная скорость перемещения игрока

using gui::GUI;

// Реализация конструктора
Engine::Engine(const EngineSettings& settings_) {
    this->settings = settings_;

    // Инициализация окна GLFW
    if (!Window::initialize(settings.display)) {
        LOG_CRITICAL("Failed to load Window");
        Window::terminate();
        throw initialize_error("Failed to load Window");
    }

    // // Устанавливаем иконку приложения
    // std::filesystem::path iconPath = engine_fs::get_icon_file(1);
    // std::unique_ptr<ImageData> icon(png::loadImage(iconPath.string(), false));
    // if (icon->getFormat() != ImageFormat::rgba8888) icon->rgb2rgba();
    // bool iconStatus = Window::setIcon(icon.get());
    // if (!iconStatus) {
    //     LOG_ERROR("Failed to set icon '{}'", iconPath.string());
    // }

    // Загрузка ассетов
    assets = new Assets();
    LOG_INFO("Loading Assets");
    AssetsLoader loader(assets);
    AssetsLoader::createDefaults(loader);
    AssetsLoader::addDefaults(loader);
    while (loader.hasNext()) {
        if (!loader.loadNext()) {
            delete assets;
            Window::terminate();
            LOG_CRITICAL("Could not to initialize assets");
            throw initialize_error("Could not to initialize assets");
        }
    }
    LOG_INFO("Assets loaded successfully");

    gui = new GUI();

    setScreen(std::shared_ptr<Screen> (new MenuScreen(this)));

    LOG_INFO("The world is loaded");
    LOG_INFO("Initialization is finished");
    Logger::getInstance().flush();
}

// Реализация деструктора
Engine::~Engine() {
    LOG_INFO("Shutting down");

    screen = nullptr;
    if (assets != nullptr) {
        delete assets;
        assets = nullptr;
    }
    Window::terminate();

    LOG_INFO("Engine finished");
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
		std::filesystem::path filename = engine_fs::get_screenshot_file("png");
		png::writeImage(filename.string(), image.get());
    }
}

// Основной цикл приложения
void Engine::mainloop() {
    LOG_INFO("Preparing systems");

    Batch2D batch(1024);

    lastTime = Window::time();
    LOG_INFO("Systems have been prepared");

    while (!Window::isShouldClose()){
        if (screen == nullptr) {
            LOG_CRITICAL("Screen is null");
            throw std::runtime_error("Screen is null");
        }

        updateTimers(); // Обновляем время и deltaTime
        updateHotkeys(); // Обрабатываем нажатия клавиш

        gui->activate(deltaTime);

        screen->update(deltaTime);
        screen->draw(deltaTime);

        gui->draw(&batch, assets);

        Window::swapBuffers(); // Показать отрендеренный кадр
        Events::pollEvents(); // Обработка событий ввода
    }
    Logger::getInstance().flush();
}

GUI* Engine::getGUI() {
	return gui;
}

EngineSettings& Engine::getSettings() {
	return settings;
}

Assets* Engine::getAssets() {
	return assets;
}

void Engine::setScreen(std::shared_ptr<Screen> screen) {
	this->screen = screen;
}
