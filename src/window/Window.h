#pragma once

#include <vector>
#include <stack>
#include <memory>

#include <glm/glm.hpp>

#include <typedefs.h>

struct GLFWwindow; // Предварительное объявление класса GLFWwindow
class ImageData;
struct GLFWmonitor;
struct DisplaySettings;

// Обертка для работы с окном приложения через GLFW
class Window {
private:
    static GLFWwindow* window;

    static DisplaySettings* settings;

    static std::stack<glm::vec4> scissorStack;
	static glm::vec4 scissorArea;

    static bool fullscreen;

    static int framerate;
    static double prevSwap;

    static bool tryToMaximize(GLFWwindow* window, GLFWmonitor* monitor);
public:
    static int posX;
	static int posY;

    static uint width;
    static uint height;

    static bool initialize(DisplaySettings* settings);
    static void terminate();

    static void setCursorMode(int mode);

    static void viewport(int x, int y, int width, int height);
    static bool isShouldClose(); // Установлен ли флаг закрытия окна
    static void setShouldClose(bool flag); // Устанавливает или снимает флаг закрытия окна
    static void swapBuffers(); // Обмен буферов
    static void setFramerate(int interval);

    static void toggleFullscreen();

	static bool isFullscreen();
    static bool isFocused();
    static bool isMaximized();
    static bool isIconified();

    static void pushScissor(glm::vec4 area);
	static void popScissor();
	static void resetScissor();

    static double time();

    static const char* getClipboardText();
    static void setClipboardText(const char* text);

    static void clear();
    static void clearDepth();
    static void setBgColor(glm::vec3 color);
    static void setBgColor(glm::vec4 color);

    static DisplaySettings* getDisplaySettings();

    static void setIcon(const ImageData* image);

    static glm::vec2 size() {
		return glm::vec2(width, height);
	}

    static std::unique_ptr<ImageData> takeScreenshot();
};
