#ifndef WINDOW_EVENTS_H_
#define WINDOW_EVENTS_H_

#include "Window.h"

typedef unsigned int uint;

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
    static void pollEvents(); // Обработка событий текущего кадра

    static bool isPressed(int keycode); // Проверяет, нажата ли клавиша в текущий момент
    static bool justPressed(int keycode); // Проверяет, была ли клавиша нажата именно в текущем кадре 

    static bool isClicked(int button); // Проверяет, нажата ли кнопка мыши в данный момент
    static bool justClicked(int button); // Проверяет, была ли кнопка мыши нажата именно в текущем кадре

    static void toggleCursor(); // Переключает режим курсора между нормальным и заблокированным состоянием
};

#endif // WINDOW_EVENTS_H
