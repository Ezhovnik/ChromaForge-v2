#ifndef SRC_ENGINE_H_
#define SRC_ENGINE_H_

#include <string>
#include <memory>
#include <stdexcept>

#include "typedefs.h"

class Assets;
class Level;

namespace gui {
    class GUI;
}

// Структура настроек движка, передающая параметры при создании Engine
struct EngineSettings {
    int displayWidth; // Ширина окна
    int displayHeight; // Высота окна
    int displaySamples;
    int displaySwapInterval;
    const char* displayTitle; // Заголовок окна

    uint chunksLoadSpeed;
    uint chunksLoadDistance;
    uint chunksPadding;

    float fogCurve;
};

void read_settings(EngineSettings& settings, std::string filename);
void write_settings(EngineSettings& settings, std::string filename);

class initialize_error : public std::runtime_error {
public:
    initialize_error(const std::string& message) : std::runtime_error(message) {}
};

// Основной класс Engine, управляющий жизненным циклом приложения
class Engine {
private:
    Assets* assets; // Менеджер ассетов (текстуры, шейдеры и т.д.)
    Level* level; // Текущий уровень (состояние мира и игрока)
    gui::GUI* gui;
    EngineSettings settings;

    uint64_t frame = 0; // Номер текущего кадра
    double lastTime = 0.0; // Время последнего кадра (для расчёта deltaTime)
    double deltaTime = 0.0; // Разница во времени между кадрами
    bool occlusion = true; // Включаем/выключаем окклюзию (отбрасывание невидимых объектов)
public:
    Engine(const EngineSettings& settings); // Конструктор
    ~Engine(); // Деструктор

    void updateTimers(); // Обновление таймеров (frame, deltaTime)
    void updateHotkeys(); // Обработка горячих клавиш
    void mainloop(); // Основной цикл приложения
};

#endif // SRC_ENGINE_H_
