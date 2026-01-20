#include <iostream>
#include <vector>
#include <ctime>
#include <exception>

#define GLEW_STATIC  // Указывает компилятору, что будем использовать статическую версию GLEW
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GLM – библиотека для работы с матрицами и векторами в OpenGL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Пользовательские заголовочные файлы
#include "window/Events.h"
#include "window/Camera.h"
#include "window/Window.h"
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
#include "frontend/hud_render.h"
#include "logger/Logger.h"
#include "logger/OpenGL_Logger.h"

// Точка спавна игрока и начальная скорость
inline constexpr glm::vec3 SPAWNPOINT = {0, 128, 0}; // Точка, где игрок появляется в мире
inline constexpr float DEFAULT_PLAYER_SPEED = 5.0f; // Начальная скорость перемещения игрока

// Пользовательская ошибка инициализации – наследуется от std::runtime_error
class initialize_error : public std::runtime_error {
public:
    initialize_error(const std::string& message) : std::runtime_error(message) {}
};

// Структура настроек движка, передающая параметры при создании Engine
struct EngineSettings {
    int displayWidth; // Ширина окна
    int displayHeight; // Высота окна
    int displaySamples;
    const char* displayTitle; // Заголовок окна

    uint chunksLoadSpeed;
    uint chunksLoadDistance;
    uint chunksPadding;
};

// Основной класс Engine, управляющий жизненным циклом приложения
class Engine {
private:
    Assets* assets; // Менеджер ассетов (текстуры, шейдеры и т.д.)
    Level* level; // Текущий уровень (состояние мира и игрока)
    EngineSettings settings;

    uint64_t frame = 0; // Номер текущего кадра
    float lastTime = 0.0f; // Время последнего кадра (для расчёта deltaTime)
    float deltaTime = 0.0f; // Разница во времени между кадрами
    bool occlusion = true; // Включаем/выключаем окклюзию (отбрасывание невидимых объектов)
public:
    Engine(const EngineSettings& settings); // Конструктор
    ~Engine(); // Деструктор

    void updateTimers(); // Обновление таймеров (frame, deltaTime)
    void updateHotkeys(); // Обработка горячих клавиш
    void mainloop(); // Основной цикл приложения
};

// Реализация конструктора
Engine::Engine(const EngineSettings& settings) {
    this->settings = settings;

    // Инициализация логгера
    Logger::getInstance().initialize();

    // Инициализация окна GLFW
    if (!Window::initialize(
            settings.displayWidth, 
            settings.displayHeight, 
            settings.displayTitle, 
            settings.displaySamples
        )) {
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
    level = world->loadLevel(player, settings.chunksLoadDistance, settings.chunksPadding);
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
    float currentTime = glfwGetTime(); // Текущее время от GLFW
    deltaTime = currentTime - lastTime; // Расчёт времени между кадрами
    lastTime = currentTime; // Обновление lastTime для следующего кадра
}

// Обработка горячих клавиш
void Engine::updateHotkeys() {
    // Закрытие окна и завершение работы
    if (Events::justPressed(GLFW_KEY_ESCAPE)) Window::setShouldClose(true);

    // Переключение курсора (включение/выключение захвата мыши)
    if (Events::justPressed(GLFW_KEY_TAB) || Events::justPressed(GLFW_KEY_E)) Events::toggleCursor();

    // Переключение окклюзии (отбрасывание невидимых объектов)
    if (Events::justPressed(GLFW_KEY_O)) occlusion = !occlusion;

    // Переключение режима отладки игрока
    if (Events::justPressed(GLFW_KEY_F3)) level->player->debug = !level->player->debug;

    if (Events::justPressed(GLFW_KEY_F8)) {
        if (level->chunks->width >= 40) level->chunks->resize(level->chunks->width / 4, level->chunks->depth / 4);
        else level->chunks->resize(level->chunks->width + 2, level->chunks->depth + 2);
    }

    // Отметка всех чанков как изменённых (для перерисовки)
    if (Events::justPressed(GLFW_KEY_F5)) {
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
    HudRenderer hud(assets);

    lastTime = glfwGetTime ();
    Window::swapInterval(1);
    LOG_INFO("Systems have been prepared");

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
        GL_CHECK(); // Проверка ошибок OpenGL
    }
}

// Точка входа в программу
int main() {
    setup_definitions();

    std::unique_ptr<Engine> engine = nullptr;

    try {
        EngineSettings settings;

        settings.displayWidth = 1280;
        settings.displayHeight = 720;
        settings.displaySamples = 4;
        settings.displayTitle = "ChromaForge";

        settings.chunksLoadSpeed = 10;
        settings.chunksLoadDistance = 12;
        settings.chunksPadding = 2;

        engine = std::make_unique<Engine>(settings);
        engine->mainloop(); // Запуск основного цикла
    } catch (const initialize_error& err) {
        LOG_CRITICAL("An initialization error occurred\n{}", err.what());
    }

    return 0;
}
