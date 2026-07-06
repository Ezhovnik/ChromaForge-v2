#pragma once

#include <string>
#include <memory>

#include <typedefs.h>
#include <delegates.h>
#include <io/engine_paths.h>
#include <content/ContentPack.h>
#include <content/content_fwd.h>
#include <content/PacksManager.h>
#include <util/ObjectsKeeper.h>
#include <core_content_defs.h>
#include <settings.h>
#include <io/settings_io.h>
#include <engine/EngineTime.h>
#include <engine/PostRunnables.h>

class Assets;
class Screen;
class EngineController;
class SettingsHandler;
struct EngineSettings;
class Level;
class Input;
class Window;

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

    std::filesystem::path resFolder = "res";
    std::filesystem::path userFolder = ".";
    std::filesystem::path scriptFile;
};

using OnWorldOpen = std::function<void(std::unique_ptr<Level>, int64_t)>;

// Основной класс Engine, управляющий жизненным циклом приложения
class Engine : public util::ObjectsKeeper {
private:
    CoreParameters params;
    EngineSettings settings;
    EnginePaths paths;

    std::unique_ptr<SettingsHandler> settingsHandler;
    std::unique_ptr<Assets> assets; // Менеджер ассетов (текстуры, модели и т.д.)
    std::shared_ptr<Screen> screen;
    std::unique_ptr<EngineController> controller;
    std::vector<ContentPack> contentPacks;
    std::unique_ptr<Content> content;
    std::unique_ptr<ResPaths> resPaths;
    std::unique_ptr<cmd::CommandsInterpreter> cmd;
    std::unique_ptr<network::Network> network;
    std::unique_ptr<Input> input;
    std::unique_ptr<Window> window;
    std::vector<std::string> basePacks;

    std::unique_ptr<gui::GUI> gui;

    PostRunnables postRunnables;

    EngineTime time;

    OnWorldOpen levelConsumer;

    bool quitSignal = false;

    void updateHotkeys(); // Обработка горячих клавиш

    void loadAssets();
    void loadControls();
    void loadSettings();
    void saveSettings();
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

    void onAssetsLoaded();

    EnginePaths& getPaths();
    ResPaths* getResPaths();
    Assets* getAssets();
	EngineSettings& getSettings();
    const Content* getContent() const;
    Content* getWriteableContent();
    std::vector<ContentPack>& getContentPacks();
    std::vector<ContentPack> getAllContentPacks();
    std::shared_ptr<Screen> getScreen();
    SettingsHandler& getSettingsHandler();
    EngineController* getController();
    std::vector<std::string>& getBasePacks();
    const CoreParameters& getCoreParameters() const;
    EngineTime& getTime();

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

    Window& getWindow() {
        return *window;
    }

    bool isHeadless() const;

    void postRunnable(const runnable& callback) {
        postRunnables.postRunnable(callback);
    }

    PacksManager createPacksManager(const io::path& worldFolder);

    void saveScreenshot();

	void setScreen(std::shared_ptr<Screen> screen);
    void setLanguage(std::string locale);
    void setLevelConsumer(OnWorldOpen levelConsumer);

    void onWorldOpen(std::unique_ptr<Level> level, int64_t localPlayer);
    void onWorldClosed();

    void quit();
    bool isQuitSignal() const;

    void loadContent();
    void resetContent();
    void loadWorldContent(const io::path& folder);
    void loadAllPacks();
};
