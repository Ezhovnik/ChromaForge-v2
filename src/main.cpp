#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "window/Window.h"
#include "window/Events.h"

int WIDTH = 1280;
int HEIGHT = 720;

int main() {
    Window::initialize(WIDTH, HEIGHT, "ChromaForge");
    Events::initialize();

    glClearColor(0, 0, 0, 1);
    while (!Window::isShouldClose()) {
        Events::pollEvents();

        if (Events::justPressed(GLFW_KEY_ESCAPE)) {
            Window::setShouldClose(true);
        }
        if (Events::justClicked(GLFW_MOUSE_BUTTON_1)) {
            glClearColor(1, 0, 0, 1);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        Window::swapBuffers();
    }

    Window::terminate();
    return 0;
}