#ifndef SRC_ENGINE_H_
#define SRC_ENGINE_H_

#include <string>
#include <stdexcept>
#include <memory>

#include "typedefs.h"
#include "settings.h"
#include "files/engine_paths.h"
#include "content/ContentPack.h"
#include "assets/Assets.h"
#include "content/Content.h"

class Screen;

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
    std::unique_ptr<Assets> assets = nullptr; // Менеджер ассетов (текстуры, модели и т.д.)
    std::shared_ptr<Screen> screen = nullptr;
    std::vector<ContentPack> contentPacks;
    EngineSettings settings;
    std::unique_ptr<Content> content = nullptr;

    EnginePaths* paths;
    std::unique_ptr<ResPaths> resPaths = nullptr;

    gui::GUI* gui;

    uint64_t frame = 0; // Номер текущего кадра
    double lastTime = 0.0; // Время последнего кадра (для расчёта deltaTime)
    double deltaTime = 0.0; // Разница во времени между кадрами
public:
    Engine(EngineSettings& settings, EnginePaths* paths); // Конструктор
    ~Engine(); // Деструктор

    void updateTimers(); // Обновление таймеров (frame, deltaTime)
    void updateHotkeys(); // Обработка горячих клавиш
    void mainloop(); // Основной цикл приложения

    EnginePaths* getPaths();
    Assets* getAssets();
	gui::GUI* getGUI();
	EngineSettings& getSettings();
    const Content* getContent() const;
    std::vector<ContentPack>& getContentPacks();

	void setScreen(std::shared_ptr<Screen> screen);
    void setLanguage(std::string locale);
    void loadContent();
};

#endif // SRC_ENGINE_H_
