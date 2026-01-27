#include "Window.h"

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Events.h"
#include "../logger/Logger.h"

GLFWwindow* Window::window = nullptr; // Статическая переменная-член класса - указатель на окно GLFW
uint Window::width = 0;
uint Window::height = 0;

// Callback-функция для обработки движения мыши
void cursor_position_callback(GLFWwindow*, double x_pos, double y_pos) {
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
void mouse_button_callback(GLFWwindow*, int button, int action, int mode) {
    if (button < 0 || button >= _MAX_MOUSE_BUTTONS) return;

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
void key_callback(GLFWwindow*, int key, int scancode, int action, int mode) {
    if (key < 0 || key >= _MOUSE_BUTTONS) return;

    if (action == GLFW_PRESS) {
        Events::_keys[key] = true;
        Events::_frames[key] = Events::_current;
    } else if (action == GLFW_RELEASE) {
        Events::_keys[key] = false;
        Events::_frames[key] = Events::_current;
    }
}

// Callback-функция для обработки изменения размера окна
void window_size_callback(GLFWwindow*, int width, int height) {
    if (width <= 0 || height <= 0) return;

    glViewport(0, 0, width, height); // Обновляем область отображения при изменении размера окна
    
    // Обновляем размеры окна у объекта окна
    Window::width = width;
    Window::height = height;
}

// Инициализация окна и OpenGL контекста
bool Window::initialize(uint width, uint height, const char* title, int samples) {
    if (!glfwInit()){ // Инициализация GLFW
        LOG_CRITICAL("GLFW initialization failed");
        return false;
    }

    // Установка версии OpenGL (3.3 Core Profile)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); // Разрешаем изменять размер окна
    glfwWindowHint(GLFW_SAMPLES, samples);

    // Создание окна GLFW
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (window == nullptr) {
        LOG_CRITICAL("Failed to create GLFW Window");
        glfwTerminate();
        return false;
    }

    // Установка созданного окна как текущего контекста OpenGL
    glfwMakeContextCurrent(window);

    // Инициализация GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        LOG_CRITICAL("Failed to initialize GLEW {}", reinterpret_cast<const char*>(glewGetErrorString(glewError)));
        return false;
    }
     // Установка области отображения (viewport) - вся область окна
    glViewport(0, 0, width, height);

    glClearColor(0.0f, 0.0f, 0.0f, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Задаём размеры окна у объекта
    Window::width = width;
    Window::height = height;

    Events::initialize();

    // Устанавливаем callback-функции GLFW
    glfwSetKeyCallback(window, key_callback); // Клавиатура
    glfwSetMouseButtonCallback(window, mouse_button_callback); // Нажатие кнопки мыши
    glfwSetCursorPosCallback(window, cursor_position_callback); // Движение мыши
    glfwSetWindowSizeCallback(window, window_size_callback); // Изменение размера окна

    LOG_INFO("Window initialized successfully: {}x{}", width, height);

    return true;
}

// Завершение работы окна и освобождение ресурсов GLFW
void Window::terminate() {
    Events::finalize();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::setCursorMode(int mode) {
    glfwSetInputMode(window, GLFW_CURSOR, mode);
}

void Window::viewport(int x, int y, int width, int height){
	glViewport(x, y, width, height);
}

// Проверка, должно ли окно закрыться
bool Window::isShouldClose() {
    return glfwWindowShouldClose(window);
}

// Установка флага закрытия окна
void Window::setShouldClose(bool flag) {
    glfwSetWindowShouldClose(window, flag);
}

void Window::swapInterval(int interval){
	glfwSwapInterval(interval);
}

// Обмен буферов
void Window::swapBuffers() {
    glfwSwapBuffers(window);
}
