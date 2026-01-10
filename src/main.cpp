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
#include "window/Window.h"
#include "window/Events.h"
#include "window/Camera.h"
#include "voxels/Chunk.h"
#include "voxels/Chunks.h"
#include "voxels/ChunksController.h"
#include "voxels/ChunksLoader.h"
#include "Assets.h"
#include "AssetsLoader.h"
#include "objects/Player.h"
#include "declarations.h"
#include "world_render.h"
#include "world/Level.h"
#include "world/World.h"
#include "hud_render.h"
#include "logger/Logger.h"
#include "logger/OpenGL_Logger.h"

// Глобальные параметры окна приложения
int WINDOW_WIDTH = 1280; // Ширина окна в пикселях
int WINDOW_HEIGHT = 720; // Высота окна в пикселях
const char* TITLE = "ChromaForge"; // Заголовок окна

// Точка спавна игрока и начальная скорость
inline constexpr glm::vec3 SPAWNPOINT = {0, 256, 0}; // Точка, где игрок появляется в мире
inline constexpr float DEFAULT_PLAYER_SPEED = 5.0f; // Начальная скорость перемещения игрока

// Пользовательская ошибка инициализации – наследуется от std::runtime_error
class initialize_error : public std::runtime_error {
    initialize_error(const std::string& message) : std::runtime_error(message) {}
};

// Структура настроек движка, передающая параметры при создании Engine
struct EngineSettings {
    int displayWidth; // Ширина окна
    int displayHeight; // Высота окна
    const char* title; // Заголовок окна
};

// Основной класс Engine, управляющий жизненным циклом приложения
class Engine {
    Assets* assets; // Менеджер ассетов (текстуры, модели и т.д.)
    Level* level; // Текущий уровень (состояние мира и игрока)

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
    // Инициализация логгера
    Logger::getInstance().initialize();

    // Инициализация окна GLFW
    if (!Window::initialize(settings.displayWidth, settings.displayHeight, settings.title)) {
        LOG_CRITICAL("Failed to load Window");
        Window::terminate();
        throw std::runtime_error("Failed to load Window");
    }

    // Инициализация логгера OpenGL
    OpenGL_Logger::getInstance().initialize(LogLevel::DEBUG);
    GL_CHECK();

    // Инициализация системы событий ввода
    Events::initialize();

    // Загрузка ассетов
    assets = new Assets();
    LOG_INFO("Loading Assets");
    AssetsLoader loader(assets);
    AssetsLoader::createDefaults(loader); // Создание наборов ассетов по умолчанию
    initialize_assets(&loader);
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
    World* world = new World("world-1", "../saves/world-1/", 42);
    Player* player = new Player(SPAWNPOINT, DEFAULT_PLAYER_SPEED, camera);
    level = world->loadLevel(player);
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
    Events::finalize();
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

    // Отметка всех чанков как изменённых (для перерисовки)
    if (Events::justPressed(GLFW_KEY_F5)) {
        for (unsigned i = 0; i < level->chunks->volume; i++) {
            Chunk* chunk = level->chunks->chunks[i];
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
    HudRenderer hud;

    lastTime = glfwGetTime();
    Window::swapInterval(1); // Включаем VSync (синхронизация с частотой обновления экрана)
    LOG_INFO("Systems have been prepared");

    while (!Window::isShouldClose()){
        updateTimers(); // Обновляем время и deltaTime
        updateHotkeys(); // Обрабатываем нажатия клавиш

        // Обновление логики уровня (перемещение игрока, столкновения и т.д.)
        level->update(deltaTime, Events::_cursor_locked);

        // Построение мешей чанков (загрузка геометрии для видимых чанков)
        int freeLoaders = level->chunksController->countFreeLoaders();
        for (int i = 0; i < freeLoaders; i++) {
            level->chunksController->_buildMeshes();
        }

        // Вычисление света для чанков (аппликация освещения)
        freeLoaders = level->chunksController->countFreeLoaders();
        for (int i = 0; i < freeLoaders; i++) {
            level->chunksController->calculateLights();
        }

        // Загрузка видимых чанков (чтение данных из файлов/памяти)
        freeLoaders = level->chunksController->countFreeLoaders();
        for (int i = 0; i < freeLoaders; i++) {
            level->chunksController->loadVisible(world->wfile);
        }

        // Рендеринг мира и HUD
        worldRenderer.draw(camera, occlusion);
        hud.draw(level, assets);
        if (level->player->debug) hud.drawDebug(level, assets, 1 / deltaTime, occlusion);

        Window::swapBuffers(); // Показать отрендеренный кадр
        Events::pollEvents(); // Обработка событий ОС и ввода
        GL_CHECK(); // Проверка ошибок OpenGL
    }
}

// Точка входа в программу
int main() {
    setup_definitions();
    try {
        Engine engine(EngineSettings{WINDOW_WIDTH, WINDOW_HEIGHT, TITLE});
        engine.mainloop(); // Запуск основного цикла
    }
    catch (const initialize_error& err) {
        LOG_CRITICAL("An initialization error occurred\n{}", err.what());
    }

    return 0;
}
