#include "Events.h"

#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Статические массивы для хранения состояние клавиш и кнопок мыши
bool* Events::_keys; // Хранит текущее состояние (нажата / не нажата)
uint* Events::_frames; // Храние номер кадра, в котором было последнее изменение состояния
uint Events::_current = 0; // Номер текущего кадра

// Переменные для отслеживания позиции мыши
float Events::deltaX = 0.0f; // Изменение положения курсора по X с последнего кадра
float Events::deltaY = 0.0f; // Изменение положения курсора по Y с последнего кадра
float Events::x = 0.0f; // Текущее положение курсора по X
float Events::y = 0.0f; // Текущее положение курсора по Y

// Флаги для упраления состоянием курсора
bool Events::_cursor_locked = false; // Режим захвата курсора
bool Events::_cursor_started = false; // Начал ли пользователь движение мышью

// Инициализация системы событий
int Events::initialize(){
    // Выделяем память: 1032 = 1024 клавиши + 8 кнопок мыши
    _keys = new bool[1032];
    _frames = new uint[1032];

    // Инициализируем массивы нулями
    memset(_keys, false, 1032 * sizeof(bool));
    memset(_frames, 0, 1032 * sizeof(uint));

    return 0;
}

void Events::finalize(){
	delete[] _keys;
	delete[] _frames;
}

// Проверяет, нажата ли клавиша в данный момент
bool Events::isPressed(int keycode) {
    if (keycode < 0 || keycode  >= _MOUSE_BUTTONS) {
        return false;
    }
    return _keys[keycode];
}

// Проверяет, была ли клавиша нажата именно в текущем кадре 
bool Events::justPressed(int keycode) {
    if (keycode < 0 || keycode  >= _MOUSE_BUTTONS) return false;

    return _keys[keycode] && _frames[keycode] == _current;
}

// Проверяет, нажата ли кнопка мыши в данный момент
bool Events::isClicked(int button) {
    if (button < 0 || button >= _MAX_MOUSE_BUTTONS) return false;

    int index = _MOUSE_BUTTONS + button;
    return _keys[index];
}

// Проверяет, была ли кнопка мыши нажата именно в текущем кадре
bool Events::justClicked(int button) {
    if (button < 0 || button >= _MAX_MOUSE_BUTTONS) return false;

    int index = _MOUSE_BUTTONS + button;
    return _keys[index] && _frames[index] == _current;
}

// Переключает режим курсора между нормальным и заблокированным состоянием
void Events::toggleCursor() {
    _cursor_locked = !_cursor_locked;
    Window::setCursorMode(_cursor_locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

// Обработка событий текущего кадра
void Events::pollEvents() {
    _current++;
    deltaX = 0.0f;
    deltaY = 0.0f;
    glfwPollEvents();
}
