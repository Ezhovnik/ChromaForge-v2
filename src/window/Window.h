#ifndef WINDOW_WINDOW_H_
#define WINDOW_WINDOW_H_

class GLFWwindow; // Предварительное объявление класса GLFWwindow

// Обертка для работы с окном приложения через GLFW
class Window {
public:
    static GLFWwindow* window;

    static int initialize(int width, int height, const char* title);
    static void terminate();

    static bool isShouldClose(); // Установлен ли флаг закрытия окна
    static void setShouldClose(bool flag); // Устанавливает или снимает флаг закрытия окна
    static void swapBuffers(); // Обмен буферов
};

#endif // WINDOW_WINDOW_H