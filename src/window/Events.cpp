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

// Константа для разделения индексов клавиш и кнопок мыши
// Клавиши: 0-1023, кнопки мыши: 1024+
const int _MOUSE_BUTTONS = 1024;

// Максимальное количество поддерживаемых кнопок мыши
// GLFW поддерживает кнопки с GLFW_MOUSE_BUTTON_1 (0) до GLFW_MOUSE_BUTTON_8 (7)
const int _MAX_MOUSE_BUTTONS = 8;

// Callback-функция для обработки движения мыши
void cursor_position_callback(GLFWwindow* window, double x_pos, double y_pos) {
    if (Events::_cursor_locked) {
        // В режиме захвата курсора накапливаем изменения позиции
        Events::deltaX += x_pos - Events::x;
        Events::deltaY += y_pos - Events::y;
    } else {
        // В обычном режиме просто отмечаем, что курсор начал движение
        Events::_cursor_started = true;
    }
    // Обновляем текущие координаты
    Events::x = x_pos;
    Events::y = y_pos;
}

// Callback-функция для обработки нажатий кнопок мыши
void mouse_button_callback(GLFWwindow* window, int button, int action, int mode) {
    if (button < 0 || button >= _MAX_MOUSE_BUTTONS) {
        return;
    }

    int index = _MOUSE_BUTTONS + button;

    if (action == GLFW_PRESS) {
        Events::_keys[index] = true;
        Events::_frames[index] = Events::_current;
    } else if (action == GLFW_RELEASE) {
        Events::_keys[index] = false;
        Events::_frames[index] = Events::_current;
    }
}

// Callback-функция для обработки нажатий клавиш клавиатуры
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key < 0 || key >= _MOUSE_BUTTONS) {
        return;
    }

    if (action == GLFW_PRESS) {
        Events::_keys[key] = true;
        Events::_frames[key] = Events::_current;
    } else if (action == GLFW_RELEASE) {
        Events::_keys[key] = false;
        Events::_frames[key] = Events::_current;
    }
}

// Callback-функция для обработки изменения размера окна
void window_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height); // Обновляем область отображения при изменении размера окна
    
    // Обновляем размеры окна у объекта окна
    Window::width = width;
    Window::height = height;
}

// Инициализация системы событий
int Events::initialize(){
    GLFWwindow* window = Window::window;

    // Выделяем память: 1032 = 1024 клавиши + 8 кнопок мыши
    _keys = new bool[1032];
    _frames = new uint[1032];

    // Инициализируем массивы нулями
    memset(_keys, false, 1032 * sizeof(bool));
    memset(_frames, 0, 1032 * sizeof(uint));

     // Устанавливаем callback-функции GLFW
    glfwSetKeyCallback(window, key_callback); // Клавиатура
    glfwSetMouseButtonCallback(window, mouse_button_callback); // Нажатие кнопки мыши
    glfwSetCursorPosCallback(window, cursor_position_callback); // Движение мыши
    glfwSetWindowSizeCallback(window, window_size_callback); // Изменение размера окна

    return 0;
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
    if (keycode < 0 || keycode  >= _MOUSE_BUTTONS) {
        return false;
    }
    return _keys[keycode] && _frames[keycode] == _current;
}

// Проверяет, нажата ли кнопка мыши в данный момент
bool Events::isClicked(int button) {
    if (button < 0 || button >= _MAX_MOUSE_BUTTONS) {
        return false;
    }
    int index = _MOUSE_BUTTONS + button;
    return _keys[index];
}

// Проверяет, была ли кнопка мыши нажата именно в текущем кадре
bool Events::justClicked(int button) {
    if (button < 0 || button >= _MAX_MOUSE_BUTTONS) {
        return false;
    }
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
