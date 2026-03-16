#ifndef SRC_ENGINE_H_
#define SRC_ENGINE_H_

#include <string>
#include <stdexcept>
#include <memory>
#include <filesystem>
#include <queue>
#include <mutex>

#include "typedefs.h"
#include "settings.h"
#include "delegates.h"
#include "files/engine_paths.h"
#include "content/ContentPack.h"
#include "assets/Assets.h"
#include "content/Content.h"
#include "files/settings_io.h"
#include "content/PacksManager.h"

class Screen;
class Batch2D;

namespace gui {
    class GUI;
}

// Пользовательская ошибка инициализации – наследуется от std::runtime_error
class initialize_error : public std::runtime_error {
public:
    initialize_error(const std::string& message) : std::runtime_error(message) {}
};

// Основной класс Engine, управляющий жизненным циклом приложения
class Engine {
private:
    EngineSettings& settings;
    SettingsHandler settingsHandler;
    EnginePaths* paths;

    std::unique_ptr<Assets> assets = nullptr; // Менеджер ассетов (текстуры, модели и т.д.)
    std::shared_ptr<Screen> screen = nullptr;
    std::vector<ContentPack> contentPacks;
    std::unique_ptr<Content> content = nullptr;

    std::unique_ptr<ResPaths> resPaths = nullptr;

    std::queue<runnable> postRunnables;
    std::recursive_mutex postRunnablesMutex;

    std::unique_ptr<gui::GUI> gui;

    uint64_t frame = 0; // Номер текущего кадра
    double lastTime = 0.0; // Время последнего кадра (для расчёта deltaTime)
    double deltaTime = 0.0; // Разница во времени между кадрами

    void updateTimers(); // Обновление таймеров (frame, deltaTime)
    void updateHotkeys(); // Обработка горячих клавиш

    void renderFrame(Batch2D& batch);

    void processPostRunnables();

    void addDefaultWorldGenerators();
    void loadAssets();
public:
    Engine(EngineSettings& settings, EnginePaths* paths); // Конструктор
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

    void postRunnable(runnable callback);

    PacksManager createPacksManager(const std::filesystem::path& worldFolder);

    void saveScreenshot();

	void setScreen(std::shared_ptr<Screen> screen);
    void setLanguage(std::string locale);

    void loadContent();
    void loadWorldContent(const std::filesystem::path& folder);
    void loadAllPacks();
};

#endif // SRC_ENGINE_H_
