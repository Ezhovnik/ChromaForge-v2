#pragma once

#include <vector>
#include <stack>
#include <memory>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <graphics/core/commons.h>

class ImageData;
struct DisplaySettings;

namespace Window {
    extern uint width;
    extern uint height;

    bool initialize(DisplaySettings* settings);
    void terminate();

    void viewport(int x, int y, int width, int height);
    void setCursorMode(int mode);
    bool isShouldClose();
    void setShouldClose(bool flag);
    void swapBuffers();
    void setFramerate(int interval);
    void toggleFullscreen();
    bool isFullscreen();
    bool isMaximized();
    bool isFocused();
    bool isIconified();

    void pushScissor(glm::vec4 area);
    void popScissor();
    void resetScissor();

    void setCursor(CursorShape shape);

    void clear();
    void clearDepth();
    void setBgColor(glm::vec3 color);
    void setBgColor(glm::vec4 color);
    double time();
    const char* getClipboardText();
    void setClipboardText(const char* text);
    DisplaySettings* getDisplaySettings();
    void setIcon(const ImageData* image);

    inline glm::vec2 size() {
		return glm::vec2(width, height);
	}

    std::unique_ptr<ImageData> takeScreenshot();
};
