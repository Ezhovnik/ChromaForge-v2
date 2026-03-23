#ifndef WINDOW_EVENTS_H_
#define WINDOW_EVENTS_H_

#include <vector>
#include <string>
#include <vector>
#include <unordered_map>

#include "Window.h"
#include "input.h"
#include "../typedefs.h"

inline constexpr short KEYS_BUFFER_SIZE = 1036;

// Система обработки событий ввода (клавиатура, мышь)
class Events {
private:
    static bool keys[KEYS_BUFFER_SIZE];
    static uint frames[KEYS_BUFFER_SIZE];
    static uint currentFrame;
    static bool cursor_drag;
public:
    // Переменные для отсеживания состояния мыши
    static glm::vec2 delta;
    static glm::vec2 cursor;
    static bool _cursor_locked; // Режим захвата курсора

    static int scroll;

    static std::vector<uint> codepoints;
    static std::vector<keycode> pressedKeys;
    static std::unordered_map<std::string, Binding> bindings;

    static void pollEvents(); // Обработка событий текущего кадра

    static bool isPressed(keycode code);
    static bool isPressed(int keycode); // Проверяет, нажата ли клавиша в текущий момент
    static bool justPressed(keycode code);
    static bool justPressed(int keycode); // Проверяет, была ли клавиша нажата именно в текущем кадре 

    static bool isClicked(mousecode button);
    static bool isClicked(int button); // Проверяет, нажата ли кнопка мыши в данный момент
    static bool justClicked(mousecode button);
    static bool justClicked(int button); // Проверяет, была ли кнопка мыши нажата именно в текущем кадре

    static void bind(std::string name, inputType type, keycode code);
	static void bind(std::string name, inputType type, mousecode code);
    static void bind(std::string name, inputType type, int code);

	static bool isActive(std::string name);
	static bool justActive(std::string name);

    static void toggleCursor(); // Переключает режим курсора между нормальным и заблокированным состоянием

    static void setKey(int key, bool b);
    static void setButton(int button, bool b);

    static void setPosition(float xpos, float ypos);

    static std::string writeBindings();
    static void loadBindings(const std::string& filename, const std::string& source);
};

#endif // WINDOW_EVENTS_H_
