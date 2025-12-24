#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GLM для работы с матрицами
#include <glm/glm.hpp>
#include <glm/ext.hpp>

// Пользовательские классы
#include "loaders/shader_loader.h"
#include "graphics/ShaderProgram.h"
#include "window/Window.h"
#include "window/Events.h"
#include "window/Camera.h"
#include "loaders/texture_loader.h"
#include "graphics/Texture.h"

// Размеры окна по умолчанию
int WIDTH = 1280;
int HEIGHT = 720;

float vertices[] = {
//    x     y     z     u     v
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

    1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
};

int main() {
    Window::initialize(WIDTH, HEIGHT, "ChromaForge"); // Инициализация окна
    Events::initialize(); // Инициализация системы событий

    // Загрузка шейдерной программы
    ShaderProgram* shader = loadShaderProgram("../Resource Files/main.vert", "../Resource Files/main.frag");
    if (shader == nullptr) {
        std::cerr << "Failed to load shader program" << std::endl;
        Window::terminate();
        return -1;
    }

    // Загрузка текстуры
    Texture* texture = loadTexture("../Resource Files/textures/grass_block_side.png");
    if (texture == nullptr) {
        std::cerr << "Failed to load texture" << std::endl;
        delete shader;
        Window::terminate();
        return -1;
    }

    // Создание VAO и VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Настройка VAO и VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Устанавливаем атрибут позиции
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);

    // Устанавливаем атрибут текстурных координат
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    glClearColor(0, 0, 0, 1);

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Camera* camera = new Camera(glm::vec3(0, 0, 1), glm::radians(40.0f));

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(0.5f, 0, 0));

    float lastTime = glfwGetTime();
    float deltaTime = 0.0f;

    float speed = 5.0f;

    float camX = 0.0f;
    float camY = 0.0f;

    // Главный игровой цикл
    while (!Window::isShouldClose()) {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Обработка ввода
        if (Events::justPressed(GLFW_KEY_ESCAPE)) {
            Window::setShouldClose(true); // Закрытие окна после нажатия ESC
        }
        if (Events::justPressed(GLFW_KEY_TAB)) {
            Events::toggleCursor();
        }
        if (Events::justClicked(GLFW_MOUSE_BUTTON_1)) {
            glClearColor(1, 0, 0, 1); // Меняем цвет заливки после нажатия ПКМ
        }

        if (Events::isPressed(GLFW_KEY_W)) {
            camera->position += camera->front * deltaTime * speed;
        }
        if (Events::isPressed(GLFW_KEY_S)) {
            camera->position -= camera->front * deltaTime * speed;
        }
        if (Events::isPressed(GLFW_KEY_A)) {
            camera->position -= camera->right * deltaTime * speed;
        }
        if (Events::isPressed(GLFW_KEY_D)) {
            camera->position += camera->right * deltaTime * speed;
        }

        if (Events::_cursor_locked) {
            camY -= Events::deltaY / Window::width;
            camX -= Events::deltaX / Window::height;

            if (camY < -glm::radians(89.0f)) {
                camY = -glm::radians(89.0f);
            } else if (camY > glm::radians(89.0f)) {
                camY = glm::radians(89.0f);
            }

            camera->rotation = glm::mat4(1.0f);
            camera->rotate(camY, camX, 0);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        // Рендеринг
        shader->use();
        shader->uniformMatrix("model", model);
        shader->uniformMatrix("projview", camera->getProjection() * camera->getView());
        texture->bind();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        Window::swapBuffers(); // Обмен буферов
        Events::pollEvents(); // Обработка событий
    }

    // Очищаем ресурсы
    delete shader;
    delete texture;

    // Удаляем OpenGL объекты
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    // Завершение работы
    Window::terminate();
    return 0;
}