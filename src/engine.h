#ifndef SRC_ENGINE_H_
#define SRC_ENGINE_H_

#include <string>
#include <stdexcept>
#include <memory>
#include <filesystem>
#include <queue>
#include <mutex>

#include <typedefs.h>
#include <delegates.h>
#include "files/engine_paths.h"
#include "content/ContentPack.h"
#include "assets/Assets.h"
#include "content/content_fwd.h"
#include "content/PacksManager.h"
#include "util/ObjectsKeeper.h"
#include <core_content_defs.h>

class Screen;
class Batch2D;
class EngineController;
class SettingsHandler;
struct EngineSettings;

namespace gui {
    class GUI;
}

namespace cmd {
    class CommandsInterpreter;
}

// Пользовательская ошибка инициализации – наследуется от std::runtime_error
class initialize_error : public std::runtime_error {
public:
    initialize_error(const std::string& message) : std::runtime_error(message) {}
};

// Основной класс Engine, управляющий жизненным циклом приложения
class Engine : public util::ObjectsKeeper {
private:
    EngineSettings& settings;
    SettingsHandler& settingsHandler;
    EnginePaths* paths;

    std::unique_ptr<Assets> assets; // Менеджер ассетов (текстуры, модели и т.д.)
    std::shared_ptr<Screen> screen;
    std::unique_ptr<EngineController> controller;
    std::vector<ContentPack> contentPacks;
    std::unique_ptr<Content> content;

    std::unique_ptr<ResPaths> resPaths;

    std::queue<runnable> postRunnables;
    std::recursive_mutex postRunnablesMutex;

    std::unique_ptr<cmd::CommandsInterpreter> interpreter;

    std::vector<std::string> basePacks;

    std::unique_ptr<gui::GUI> gui;

    uint64_t frame = 0; // Номер текущего кадра
    double lastTime = 0.0; // Время последнего кадра (для расчёта deltaTime)
    double deltaTime = 0.0; // Разница во времени между кадрами

    void updateTimers(); // Обновление таймеров (frame, deltaTime)
    void updateHotkeys(); // Обработка горячих клавиш

    void renderFrame(Batch2D& batch);

    void processPostRunnables();

    void loadAssets();
    void loadControls();
    void loadSettings();
    void saveSettings();
public:
    Engine(EngineSettings& settings, SettingsHandler& settingsHandler, EnginePaths* paths); // Конструктор
    ~Engine(); // Деструктор

    void mainloop(); // Основной цикл приложения

    void onAssetsLoaded();

    EnginePaths* getPaths();
    ResPaths* getResPaths();
    Assets* getAssets();
	gui::GUI* getGUI();
	EngineSettings& getSettings();
    const Content* getContent() const;
    std::vector<ContentPack>& getContentPacks();
    std::shared_ptr<Screen> getScreen();
    double getDeltaTime() const;
    SettingsHandler& getSettingsHandler();
    EngineController* getController();
    std::vector<std::string>& getBasePacks();
    cmd::CommandsInterpreter* getCommandsInterpreter();

    void postRunnable(const runnable& callback);

    PacksManager createPacksManager(const std::filesystem::path& worldFolder);

    void saveScreenshot();

	void setScreen(std::shared_ptr<Screen> screen);
    void setLanguage(std::string locale);

    void loadContent();
    void resetContent();
    void loadWorldContent(const std::filesystem::path& folder);
    void loadAllPacks();
};

#endif // SRC_ENGINE_H_
