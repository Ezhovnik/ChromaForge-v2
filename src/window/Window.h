#ifndef WINDOW_WINDOW_H_
#define WINDOW_WINDOW_H_

class GLFWwindow; // Предварительное объявление класса GLFWwindow

// Обертка для работы с окном приложения через GLFW
class Window {
public:
    // TODO: Перенести GLFWwindow* window в приватную зону
    static GLFWwindow* window;

    static int width;
    static int height;

    static bool initialize(int width, int height, const char* title);
    static void terminate();

    static void setCursorMode(int mode);

    static void viewport(int x, int y, int width, int height);
    static bool isShouldClose(); // Установлен ли флаг закрытия окна
    static void setShouldClose(bool flag); // Устанавливает или снимает флаг закрытия окна
    static void swapBuffers(); // Обмен буферов
    static void swapInterval(int interval);
};

#endif // WINDOW_WINDOW_H
