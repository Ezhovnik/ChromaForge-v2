#ifndef SRC_ENGINE_H_
#define SRC_ENGINE_H_

#include <string>
#include <stdexcept>
#include <memory>

#include "typedefs.h"
#include "settings.h"

class Assets;
class Screen;
class Content;
class EnginePaths;

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
    Assets* assets; // Менеджер ассетов (текстуры, модели и т.д.)
    std::shared_ptr<Screen> screen = nullptr;
    EngineSettings settings;
    Content* content;
    EnginePaths* paths;

    gui::GUI* gui;

    uint64_t frame = 0; // Номер текущего кадра
    double lastTime = 0.0; // Время последнего кадра (для расчёта deltaTime)
    double deltaTime = 0.0; // Разница во времени между кадрами
public:
    Engine(EngineSettings& settings, EnginePaths* paths, Content* content); // Конструктор
    ~Engine(); // Деструктор

    void updateTimers(); // Обновление таймеров (frame, deltaTime)
    void updateHotkeys(); // Обработка горячих клавиш
    void mainloop(); // Основной цикл приложения

    EnginePaths* getPaths();
    Assets* getAssets();
	gui::GUI* getGUI();
	EngineSettings& getSettings();
    const Content* getContent() const;

	void setScreen(std::shared_ptr<Screen> screen);
};

#endif // SRC_ENGINE_H_
