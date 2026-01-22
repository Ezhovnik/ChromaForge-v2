#include "engine.h"

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
#include "logger/Logger.h"
#include "logger/OpenGL_Logger.h"

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

    // Инициализация логгера OpenGL
    OpenGL_Logger::getInstance().initialize(LogLevel::DEBUG);
    GL_CHECK();

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
    LOG_INFO("Loading world");

    // Создание камеры, мира и игрока
    Camera* camera = new Camera(SPAWNPOINT, glm::radians(90.0f));
    World* world = new World("world-1", "../saves/world-1/", 42);
    Player* player = new Player(SPAWNPOINT, DEFAULT_PLAYER_SPEED, camera);
    level = world->loadLevel(player, settings);

    gui = new GUI();
    LOG_INFO("The world is loaded");
    LOG_INFO("Initialization is finished");
}

// Реализация деструктора
Engine::~Engine() {
    LOG_INFO("World saving");
    World* world = level->world;
    world->write(level); // Сохранение текущего состояния уровня в файл
    delete level;
    delete world;
    LOG_INFO("The world has been successfully saved");

    LOG_INFO("Shutting down");
    delete assets;
    OpenGL_Logger::getInstance().finalize();
    Window::terminate();
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
    // Переключение окклюзии (отбрасывание невидимых объектов)
    if (Events::justPressed(keycode::O)) occlusion = !occlusion;

    // Переключение режима отладки игрока
    if (Events::justPressed(keycode::F3)) level->player->debug = !level->player->debug;

    // Отметка всех чанков как изменённых (для перерисовки)
    if (Events::justPressed(keycode::F5)) {
        for (uint i = 0; i < level->chunks->volume; ++i) {
            std::shared_ptr<Chunk> chunk = level->chunks->chunks[i];
            if (chunk != nullptr && chunk->isReady()) chunk->setModified(true);
        }
    }
}

// Основной цикл приложения
void Engine::mainloop() {
    LOG_INFO("Preparing systems");

    Camera* camera = level->player->camera;
    WorldRenderer worldRenderer(level, assets);
    HudRenderer hud(gui, level, assets);
    Batch2D batch(1024);

    lastTime = Window::time();
    LOG_INFO("Systems have been prepared");

    while (!Window::isShouldClose()){
        updateTimers(); // Обновляем время и deltaTime
        updateHotkeys(); // Обрабатываем нажатия клавиш

        bool inputLocked = hud.isPause() || hud.isInventoryOpen() || gui->isFocusCaught();
        level->updatePlayer(deltaTime, !inputLocked, hud.isPause(), !inputLocked);
        level->update();
        level->chunksController->update(settings.chunks.loadSpeed);

        // Рендеринг мира и HUD
        worldRenderer.draw(camera, occlusion, 16.0f / (float)settings.chunks.loadDistance, settings.fogCurve);
        hud.draw();
        if (level->player->debug) hud.drawDebug(1 / deltaTime, occlusion);

        gui->activate(deltaTime);
        gui->draw(&batch, assets);

        Window::swapBuffers(); // Показать отрендеренный кадр
        Events::pollEvents(); // Обработка событий ОС и ввода
        GL_CHECK(); // Проверка ошибок OpenGL
    }
}
