#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "loaders/shader_loader.h"
#include "graphics/ShaderProgram.h"
#include "window/Window.h"
#include "window/Events.h"
#include "loaders/texture_loader.h"
#include "graphics/Texture.h"

int WIDTH = 1280;
int HEIGHT = 720;

float vertices[] = {
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
};

int main() {
    Window::initialize(WIDTH, HEIGHT, "ChromaForge");
    Events::initialize();

    glClearColor(0, 0, 0, 1);

    ShaderProgram* shader = loadShaderProgram("../Resource Files/main.vert", "../Resource Files/main.frag");
    if (shader == nullptr) {
        std::cerr << "Failed to load shader program" << std::endl;
        Window::terminate();
        return -1;
    }

    Texture* texture = loadTexture("../Resource Files/textures/grass_block_side.png");
    if (texture == nullptr) {
        std::cerr << "Failed to load texture" << std::endl;
        delete shader;
        Window::terminate();
        return -1;
    }

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    while (!Window::isShouldClose()) {
        Events::pollEvents();

        if (Events::justPressed(GLFW_KEY_ESCAPE)) {
            Window::setShouldClose(true);
        }
        if (Events::justClicked(GLFW_MOUSE_BUTTON_1)) {
            glClearColor(1, 0, 0, 1);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        shader->use();
        texture->bind();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        Window::swapBuffers();
    }
    delete shader;
    delete texture;
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    Window::terminate();
    return 0;
}