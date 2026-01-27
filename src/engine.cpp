#include "engine.h"

#include <vector>
#include <ctime>
#include <exception>
#include <memory>

#define GLEW_STATIC  // Указывает компилятору, что будем использовать статическую версию GLEW
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GLM – библиотека для работы с матрицами и векторами в OpenGL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Пользовательские заголовочные файлы
#include "window/Window.h"
#include "window/Events.h"
#include "window/Camera.h"
#include "window/input.h"
#include "voxels/Chunk.h"
#include "voxels/Chunks.h"
#include "voxels/ChunksController.h"
#include "assets/Assets.h"
#include "assets/AssetsLoader.h"
#include "objects/Player.h"
#include "definitions.h"
#include "frontend/world_render.h"
#include "world/Level.h"
#include "world/World.h"
#include "frontend/hud.h"
#include "logger/Logger.h"

// Точка спавна игрока и начальная скорость
inline constexpr glm::vec3 SPAWNPOINT = {0, 256, 0}; // Точка, где игрок появляется в мире
inline constexpr float DEFAULT_PLAYER_SPEED = 5.0f; // Начальная скорость перемещения игрока

// Реализация конструктора
Engine::Engine(const EngineSettings& settings) {
    this->settings = settings;

    // Инициализация логгера
    Logger::getInstance().initialize();

    // Инициализация окна GLFW
    if (!Window::initialize(settings.displayWidth, settings.displayHeight, settings.title, settings.displaySamples)) {
        LOG_CRITICAL("Failed to load Window");
        Window::terminate();
        throw initialize_error("Failed to load Window");
    }

    // Загрузка ассетов
    assets = new Assets();
    LOG_INFO("Loading Assets");
    AssetsLoader loader(assets);
    AssetsLoader::createDefaults(loader); // Создание наборов ассетов по умолчанию
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
    LOG_INFO("Loading world");

    // Создание камеры, мира и игрока
    Camera* camera = new Camera(SPAWNPOINT, glm::radians(90.0f));
    World* world = new World("world-1", "../build/saves/world-1/", 42);
    Player* player = new Player(SPAWNPOINT, DEFAULT_PLAYER_SPEED, camera);
    level = world->loadLevel(player, settings.chunksLoadDistance, settings.chunksPadding);
    LOG_INFO("The world is loaded");
    LOG_INFO("Initialization is finished");
    Logger::getInstance().flush();
}

// Реализация деструктора
Engine::~Engine() {
    LOG_INFO("World saving");
    World* world = level->world;
    world->write(level); // Сохранение текущего состояния уровня в файл
    delete level;
    delete world;
    LOG_INFO("The world has been successfully saved");
    Logger::getInstance().flush();

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
    // Закрытие окна и завершение работы
    if (Events::justPressed(keycode::ESCAPE)) Window::setShouldClose(true);

    // Переключение курсора (включение/выключение захвата мыши)
    if (Events::justPressed(keycode::E)) Events::toggleCursor();

    // Переключение окклюзии (отбрасывание невидимых объектов)
    if (Events::justPressed(keycode::O)) occlusion = !occlusion;

    // Переключение режима отладки игрока
    if (Events::justPressed(keycode::F3)) level->player->debug = !level->player->debug;

    // Отметка всех чанков как изменённых (для перерисовки)
    if (Events::justPressed(keycode::F5)) {
        for (uint i = 0; i < level->chunks->volume; i++) {
            std::shared_ptr<Chunk> chunk = level->chunks->chunks[i];
            if (chunk != nullptr && chunk->isReady()) chunk->setModified(true);
        }
    }
}

// Основной цикл приложения
void Engine::mainloop() {
    LOG_INFO("Preparing systems");

    Camera* camera = level->player->camera;
    World* world = level->world;
    WorldRenderer worldRenderer(level, assets);
    HudRenderer hud(assets);

    lastTime = Window::time();
    Window::swapInterval(settings.displaySwapInterval); // Включаем VSync (синхронизация с частотой обновления экрана)
    LOG_INFO("Systems have been prepared");

    Logger::getInstance().flush();

    while (!Window::isShouldClose()){
        updateTimers(); // Обновляем время и deltaTime
        updateHotkeys(); // Обрабатываем нажатия клавиш

        // Обновление логики уровня (перемещение игрока, столкновения и т.д.)
        level->update(deltaTime, Events::_cursor_locked);
        level->chunksController->update(settings.chunksLoadSpeed);

        // Рендеринг мира и HUD
        worldRenderer.draw(camera, occlusion);
        hud.draw(level);
        if (level->player->debug) hud.drawDebug(level, 1 / deltaTime, occlusion);

        Window::swapBuffers(); // Показать отрендеренный кадр
        Events::pollEvents(); // Обработка событий ОС и ввода
    }
}
