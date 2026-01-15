#include "Window.h"

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../logger/Logger.h"

GLFWwindow* Window::window = nullptr; // Статическая переменная-член класса - указатель на окно GLFW
int Window::width = 0;
int Window::height = 0;

// Инициализация окна и OpenGL контекста
bool Window::initialize(int width, int height, const char* title) {
    if (!glfwInit()){ // Инициализация GLFW
        LOG_CRITICAL("GLFW initialization failed");
        return false;
    }

    // Установка версии OpenGL (3.3 Core Profile)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); // Разрешаем изменять размер окна
    glfwWindowHint(GLFW_SAMPLES, 16);

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

    glClearColor(0.0f,0.0f,0.0f,1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Задаём размеры окна у объекта
    Window::width = width;
    Window::height = height;

    LOG_INFO("Window initialized successfully: {}x{}", width, height);

    return true;
}

// Завершение работы окна и освобождение ресурсов GLFW
void Window::terminate() {
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
