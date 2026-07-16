#pragma once

#include <string>
#include <memory>

#include <typedefs.h>
#include <delegates.h>
#include <io/engine_paths.h>
#include <util/ObjectsKeeper.h>
#include <core_content_defs.h>
#include <settings.h>
#include <io/settings_io.h>
#include <engine/EngineTime.h>
#include <engine/PostRunnables.h>

class Assets;
class Screen;
class EngineController;
class Level;
class Input;
class Window;
class ContentControl;
struct Project;

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
}

// Пользовательская ошибка инициализации – наследуется от std::runtime_error
class initialize_error : public std::runtime_error {
public:
    initialize_error(const std::string& message) : std::runtime_error(message) {}
};

struct CoreParameters {
    bool headless = false;
    bool testMode = false;

    std::filesystem::path resFolder = "res";
    std::filesystem::path userFolder = ".";
    std::filesystem::path scriptFile;
    std::filesystem::path projectFolder;
};

using OnWorldOpen = std::function<void(std::unique_ptr<Level>, int64_t)>;

// Основной класс Engine, управляющий жизненным циклом приложения
class Engine : public util::ObjectsKeeper {
private:
    CoreParameters params;
    EngineSettings settings;
    EnginePaths paths;

    std::unique_ptr<Project> project;
    std::unique_ptr<SettingsHandler> settingsHandler;
    std::unique_ptr<Assets> assets; // Менеджер ассетов (текстуры, модели и т.д.)
    std::shared_ptr<Screen> screen;
    std::unique_ptr<EngineController> controller;
    std::unique_ptr<ContentControl> content;
    std::unique_ptr<cmd::CommandsInterpreter> cmd;
    std::unique_ptr<network::Network> network;
    std::unique_ptr<Input> input;
    std::unique_ptr<Window> window;

    std::unique_ptr<gui::GUI> gui;
    std::unique_ptr<devtools::Editor> editor;

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

    void updateFrontend();
    void renderFrame();
    void nextFrame();

    EnginePaths& getPaths();
    ResPaths& getResPaths();
    Assets* getAssets();
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

    network::Network& getNetwork() {
        return *network;
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

    bool isHeadless() const;

    void postRunnable(const runnable& callback) {
        postRunnables.postRunnable(callback);
    }

    void saveScreenshot();

	void setScreen(std::shared_ptr<Screen> screen);
    void setLevelConsumer(OnWorldOpen levelConsumer);

    void onWorldOpen(std::unique_ptr<Level> level, int64_t localPlayer);
    void onWorldClosed();

    void quit();
    bool isQuitSignal() const;
};
