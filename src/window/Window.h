#ifndef WINDOW_WINDOW_H_
#define WINDOW_WINDOW_H_

#include <stack>
#include <vector>

#include <glm/glm.hpp>

#include "../typedefs.h"
#include "../settings.h"

class GLFWwindow;
class ImageData;

// Обертка для работы с окном приложения через GLFW
class Window {
private:
    static GLFWwindow* window;

    static std::stack<glm::vec4> scissorStack;
    static glm::vec4 scissorArea;
public:
    static uint width;
    static uint height;

    static bool initialize(DisplaySettings& settings);
    static void terminate();

    static void setCursorMode(int mode);

    static void viewport(int x, int y, int width, int height);
    static bool isShouldClose(); // Установлен ли флаг закрытия окна
    static void setShouldClose(bool flag); // Устанавливает или снимает флаг закрытия окна
    static void swapBuffers(); // Обмен буферов
    static void swapInterval(int interval);

    static void pushScissor(glm::vec4 area);
    static void popScissor();
    static void resetScissor();

    static void clear();
    static void setBgColor(glm::vec3 color);

    static double time();

    static glm::vec2 size() {
		return glm::vec2(width, height);
	}

    static ImageData* takeScreenshot();
};

#endif // WINDOW_WINDOW_H
