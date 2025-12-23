#include "Window.h"

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

GLFWwindow* Window::window; // Статическая переменная-член класса - указатель на окно GLFW

// Инициализация окна и OpenGL контекста
int Window::initialize(int width, int height, const char* title) {
    glfwInit(); // Инициализация GLFW

    // Установка версии OpenGL (3.3 Core Profile)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); // Можно изменять размер окна

    // Создание окна GLFW
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failde to create GLFW Window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Установка созданного окна как текущего контекста OpenGL
    glfwMakeContextCurrent(window);

    // Инициализация GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

     // Установка области отображения (viewport) - вся область окна
    glViewport(0, 0, width, height);

    return 0;
}

// Завершение работы окна и освобождение ресурсов GLFW
void Window::terminate() {
    glfwTerminate();
}

// Проверка, должно ли окно закрыться
bool Window::isShouldClose() {
    return glfwWindowShouldClose(window);
}

// Установка флага закрытия окна
void Window::setShouldClose(bool flag) {
    glfwSetWindowShouldClose(window, flag);
}

// Обмен буферов
void Window::swapBuffers() {
    glfwSwapBuffers(window);
}