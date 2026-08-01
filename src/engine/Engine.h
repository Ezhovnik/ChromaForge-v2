#pragma once

#include <string>
#include <memory>

#include <util/ObjectsKeeper.h>
#include <core_content_defs.h>
#include <settings.h>
#include <engine/EngineTime.h>
#include <engine/PostRunnables.h>
#include <engine/CoreParameters.h>

class Assets;
class AssetsLoader;
class AssetsManagement;
class Screen;
class EngineController;
class Level;
class Input;
class Window;
class ContentControl;
struct Project;
class WindowControl;
class SettingsHandler;
class EnginePaths;
class ResPaths;

namespace gui {
    class GUI;
}

namespace cmd {
    class CommandsInterpreter;
}

namespace network {
    class Network;
}

namespace devtools {
    class Editor;
    class DebuggingServer;
}

// Пользовательская ошибка инициализации – наследуется от std::runtime_error
class initialize_error : public std::runtime_error {
public:
    initialize_error(const std::string& message) : std::runtime_error(message) {}
};

using OnWorldOpen = std::function<void(std::unique_ptr<Level>, int64_t)>;

// Основной класс Engine, управляющий жизненным циклом приложения
class Engine : public util::ObjectsKeeper {
private:
    CoreParameters params;
    EngineSettings settings;
    std::unique_ptr<EnginePaths> paths;
    std::unique_ptr<Project> project;
    std::unique_ptr<SettingsHandler> settingsHandler;
    std::unique_ptr<AssetsManagement> assets;
    std::shared_ptr<Screen> screen;
    std::unique_ptr<EngineController> controller;
    std::unique_ptr<ContentControl> content;
    std::unique_ptr<cmd::CommandsInterpreter> cmd;
    std::unique_ptr<network::Network> network;
    std::unique_ptr<Input> input;
    std::unique_ptr<Window> window;

    std::unique_ptr<gui::GUI> gui;
    std::unique_ptr<devtools::Editor> editor;
    std::unique_ptr<devtools::DebuggingServer> debuggingServer;
    std::unique_ptr<WindowControl> windowControl;

    PostRunnables postRunnables;

    EngineTime time;

    OnWorldOpen levelConsumer;

    bool quitSignal = false;

    void updateHotkeys(); // Обработка горячих клавиш

    void loadAssets();
    void loadProject();
    void loadControls();
    void loadSettings();
    void saveSettings();

    void initializeClient();
    void onContentLoad();
public:
    Engine(); // Конструктор
    ~Engine(); // Деструктор

    static Engine& getInstance();
    void initialize(CoreParameters coreParameters);
    void close();
    static void terminate();

    void run();

    void postUpdate();

    void applicationSpark();
    void updateFrontend();
    void renderFrame();
    void nextFrame(bool waitForRefresh);
    void startPauseLoop();

    EnginePaths& getPaths();
    ResPaths& getResPaths();
    Assets* getAssets();
    AssetsLoader& acquireBackgroundLoader();
	EngineSettings& getSettings();
    std::shared_ptr<Screen> getScreen();
    SettingsHandler& getSettingsHandler();
    EngineController* getController();
    const CoreParameters& getCoreParameters() const;
    EngineTime& getTime();
    ContentControl& getContentControl();

    gui::GUI& getGUI() {
        return *gui;
    }

    Input& getInput() {
        return *input;
    }

    network::Network* getNetwork() {
        return network.get();
    }

    cmd::CommandsInterpreter& getCmd() {
        return *cmd;
    }

    devtools::Editor& getEditor() {
        return *editor;
    }

    Window& getWindow() {
        return *window;
    }

    const Project& getProject() {
        return *project;
    }

    devtools::DebuggingServer* getDebuggingServer() {
        return debuggingServer.get();
    }

    void detachDebugger();

    bool isHeadless() const;

    void postRunnable(const runnable& callback) {
        postRunnables.postRunnable(callback);
    }

	void setScreen(std::shared_ptr<Screen> screen);
    void setLevelConsumer(OnWorldOpen levelConsumer);

    void onWorldOpen(std::unique_ptr<Level> level, int64_t localPlayer);
    void onWorldClosed();

    void quit();
    bool isQuitSignal() const;
};
