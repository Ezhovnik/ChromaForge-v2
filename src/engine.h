#pragma once

#include <string>
#include <stdexcept>
#include <memory>
#include <filesystem>
#include <queue>
#include <mutex>

#include <typedefs.h>
#include <delegates.h>
#include <files/engine_paths.h>
#include <content/ContentPack.h>
#include <assets/Assets.h>
#include <content/content_fwd.h>
#include <content/PacksManager.h>
#include <util/ObjectsKeeper.h>
#include <core_content_defs.h>
#include <settings.h>
#include <files/settings_io.h>
#include <EngineTime.h>

class Screen;
class EngineController;
class SettingsHandler;
struct EngineSettings;
class Level;

namespace gui {
    class GUI;
}

namespace cmd {
    class CommandsInterpreter;
}

namespace network {
    class Network;
}

// Пользовательская ошибка инициализации – наследуется от std::runtime_error
class initialize_error : public std::runtime_error {
public:
    initialize_error(const std::string& message) : std::runtime_error(message) {}
};

struct CoreParameters {
    bool headless = false;
    bool testMode = false;

    std::filesystem::path resFolder {"res"};
    std::filesystem::path userFolder {"."};
    std::filesystem::path scriptFile;
};

// Основной класс Engine, управляющий жизненным циклом приложения
class Engine : public util::ObjectsKeeper {
private:
    CoreParameters params;
    EngineSettings settings;
    SettingsHandler settingsHandler;
    EnginePaths paths;

    std::unique_ptr<Assets> assets; // Менеджер ассетов (текстуры, модели и т.д.)
    std::shared_ptr<Screen> screen;
    std::unique_ptr<EngineController> controller;
    std::vector<ContentPack> contentPacks;
    std::unique_ptr<Content> content;

    std::unique_ptr<ResPaths> resPaths;

    std::queue<runnable> postRunnables;
    std::recursive_mutex postRunnablesMutex;

    std::unique_ptr<cmd::CommandsInterpreter> interpreter;

    std::unique_ptr<network::Network> network;

    std::vector<std::string> basePacks;

    std::unique_ptr<gui::GUI> gui;

    EngineTime time;

    consumer<std::unique_ptr<Level>> levelConsumer;

    bool quitSignal = false;

    void updateHotkeys(); // Обработка горячих клавиш

    void processPostRunnables();

    void loadAssets();
    void loadControls();
    void loadSettings();
    void saveSettings();
public:
    Engine(CoreParameters coreParameters); // Конструктор
    ~Engine(); // Деструктор

    void run();

    void postUpdate();

    void updateFrontend();
    void renderFrame();
    void nextFrame();

    void onAssetsLoaded();

    EnginePaths& getPaths();
    ResPaths* getResPaths();
    Assets* getAssets();
	gui::GUI* getGUI();
	EngineSettings& getSettings();
    const Content* getContent() const;
    std::vector<ContentPack>& getContentPacks();
    std::vector<ContentPack> getAllContentPacks();
    std::shared_ptr<Screen> getScreen();
    SettingsHandler& getSettingsHandler();
    EngineController* getController();
    std::vector<std::string>& getBasePacks();
    cmd::CommandsInterpreter* getCommandsInterpreter();
    network::Network& getNetwork();
    const CoreParameters& getCoreParameters() const;
    EngineTime& getTime();

    bool isHeadless() const;

    void postRunnable(const runnable& callback);

    PacksManager createPacksManager(const std::filesystem::path& worldFolder);

    void saveScreenshot();

	void setScreen(std::shared_ptr<Screen> screen);
    void setLanguage(std::string locale);
    void setLevelConsumer(consumer<std::unique_ptr<Level>> levelConsumer);

    void onWorldOpen(std::unique_ptr<Level> level);
    void onWorldClosed();

    void quit();
    bool isQuitSignal() const;

    void loadContent();
    void resetContent();
    void loadWorldContent(const std::filesystem::path& folder);
    void loadAllPacks();
};
