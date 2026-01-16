#ifndef WINDOW_EVENTS_H_
#define WINDOW_EVENTS_H_

#include "Window.h"

#include "../typedefs.h"

// Константа для разделения индексов клавиш и кнопок мыши
// Клавиши: 0-1023, кнопки мыши: 1024+
const int _MOUSE_BUTTONS = 1024;

// Максимальное количество поддерживаемых кнопок мыши
// GLFW поддерживает кнопки с GLFW_MOUSE_BUTTON_1 (0) до GLFW_MOUSE_BUTTON_8 (7)
const int _MAX_MOUSE_BUTTONS = 8;

// Система обработки событий ввода (клавиатура, мышь)
class Events {
public:
    // Поля для хранения состояния ввода
    static bool* _keys; // Состояние клавиш и кнопок мыши
    static uint* _frames; // Хранит номер кадра, в котором клавиша/кнопка была нажата/отпущена
    static uint _current; // Номер текущего кадра

    // Переменные для отсеживания состояния мыши
    static float deltaX; // Изменение положения курсора по X с последнего кадра
    static float deltaY; // Изменение положения курсора по Y с последнего кадра
    static float x; // Текущее положение курсора по X
    static float y; // Текущее положение курсора по Y
    static bool _cursor_locked; // Режим захвата курсора
    static bool _cursor_started; // Начал ли пользователь движение мышью

    // Методы инициализации и обновления
    static int initialize(); // Инициализация системы событий
    static void finalize();
    static void pollEvents(); // Обработка событий текущего кадра

    static bool isPressed(int keycode); // Проверяет, нажата ли клавиша в текущий момент
    static bool justPressed(int keycode); // Проверяет, была ли клавиша нажата именно в текущем кадре 

    static bool isClicked(int button); // Проверяет, нажата ли кнопка мыши в данный момент
    static bool justClicked(int button); // Проверяет, была ли кнопка мыши нажата именно в текущем кадре

    static void toggleCursor(); // Переключает режим курсора между нормальным и заблокированным состоянием
};

#endif // WINDOW_EVENTS_H
