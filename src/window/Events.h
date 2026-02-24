#ifndef WINDOW_EVENTS_H_
#define WINDOW_EVENTS_H_

#include <vector>
#include <string>
#include <vector>
#include <unordered_map>

#include "Window.h"
#include "input.h"
#include "../typedefs.h"

constexpr short _MOUSE_KEYS_OFFSET = 1024;
constexpr short KEYS_BUFFER_SIZE = 1036;

// Система обработки событий ввода (клавиатура, мышь)
class Events {
public:
    // Поля для хранения состояния ввода
    static bool _keys[KEYS_BUFFER_SIZE]; // Состояние клавиш и кнопок мыши
    static uint _frames[KEYS_BUFFER_SIZE]; // Хранит номер кадра, в котором клавиша/кнопка была нажата/отпущена
    static uint _current; // Номер текущего кадра

    // Переменные для отсеживания состояния мыши
    static glm::vec2 delta;
    static glm::vec2 cursor;
    static bool cursor_drag;
    static bool _cursor_locked; // Режим захвата курсора

    static int scroll;

    static std::vector<uint> codepoints;
    static std::vector<int> pressedKeys;
    static std::unordered_map<std::string, Binding> bindings;

    static void pollEvents(); // Обработка событий текущего кадра

    static bool isPressed(int keycode); // Проверяет, нажата ли клавиша в текущий момент
    static bool justPressed(int keycode); // Проверяет, была ли клавиша нажата именно в текущем кадре 

    static bool isClicked(int button); // Проверяет, нажата ли кнопка мыши в данный момент
    static bool justClicked(int button); // Проверяет, была ли кнопка мыши нажата именно в текущем кадре

    static void bind(std::string name, inputType type, int code);
	static bool isActive(std::string name);
	static bool justActive(std::string name);

    static void toggleCursor(); // Переключает режим курсора между нормальным и заблокированным состоянием

    static void setKey(int key, bool b);
    static void setButton(int button, bool b);

    static void setPosition(float xpos, float ypos);
};

#endif // WINDOW_EVENTS_H_
