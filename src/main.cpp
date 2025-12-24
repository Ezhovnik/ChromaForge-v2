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
#include "graphics/Mesh.h"
#include "voxels/voxel.h"
#include "voxels/Chunk.h"
#include "graphics/VoxelRenderer.h"

// Размеры окна по умолчанию
int WIDTH = 1280;
int HEIGHT = 720;

int main() {
    Window::initialize(WIDTH, HEIGHT, "ChromaForge"); // Инициализация окна
    Events::initialize(); // Инициализация системы событий

    // Загрузка шейдерной программы
    ShaderProgram* shader = loadShaderProgram("../res/main.vert", "../res/main.frag");
    if (shader == nullptr) {
        std::cerr << "Failed to load shader program" << std::endl;
        Window::terminate();
        return -1;
    }

    // Загрузка текстуры
    Texture* texture = loadTexture("../res/textures/atlas.png");
    if (texture == nullptr) {
        std::cerr << "Failed to load texture" << std::endl;
        delete shader;
        Window::terminate();
        return -1;
    }

    // Создание чанка
    VoxelRenderer renderer(1024 * 1024 * 8); // Инициализация воксельного рендерера
    Chunk* chunk = new Chunk(); // Инициализация чанка
    Mesh* mesh = renderer.render(chunk); // Рендеринг чанка в графический меш

    glClearColor(0, 0, 0, 1);

    glEnable(GL_DEPTH_TEST); // Включение теста глубины

    // Включение отсечения задних граней
    glEnable(GL_CULL_FACE); 
    glCullFace(GL_BACK);
    
    glEnable(GL_BLEND); // Включение смешивания цветов
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Режим смешивания для прозрачности

    Camera* camera = new Camera(glm::vec3(0, 0, 1), glm::radians(70.0f)); // Создание камеры

    // Матрица модели
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(0.5f, 0, 0));

    // Переменные для отслеживания времени
    float lastTime = glfwGetTime();
    float deltaTime = 0.0f;

    // Параметры управления
    float speed = 5.0f; // Скорость движения камеры
    float camX = 0.0f; // Угол поворота камеры по горизонтали
    float camY = 0.0f; // Угол поворота камеры по вертикали

    // Главный игровой цикл
    while (!Window::isShouldClose()) {
        // Расчет времени между кадрами
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Обработка ввода
        if (Events::justPressed(GLFW_KEY_ESCAPE)) {
            Window::setShouldClose(true); // Закрытие окна после нажатия ESC
        }
        if (Events::justPressed(GLFW_KEY_TAB)) {
            Events::toggleCursor(); // Переключение режима курсора (заблокирован/разблокирован)
        }
        if (Events::justClicked(GLFW_MOUSE_BUTTON_1)) {
            glClearColor(1, 0, 0, 1); // Меняем цвет заливки после нажатия ПКМ
        }

        // Управление движением камеры с помощью WASD
        if (Events::isPressed(GLFW_KEY_W)) {
            camera->position += camera->front * deltaTime * speed; // Вперёд (W)
        }
        if (Events::isPressed(GLFW_KEY_S)) {
            camera->position -= camera->front * deltaTime * speed; // Назад (S)
        }
        if (Events::isPressed(GLFW_KEY_A)) {
            camera->position -= camera->right * deltaTime * speed; // Влево (A)
        }
        if (Events::isPressed(GLFW_KEY_D)) {
            camera->position += camera->right * deltaTime * speed; // Вправо (D)
        }

        // Управление поворотом камеры мышью (только в заблокированном режиме)
        if (Events::_cursor_locked) {
            // Обновление углов поворота на основе движения мыши
            camY -= Events::deltaY / Window::width; // Вертикальный поворот
            camX -= Events::deltaX / Window::height; // Горизонтальный поворот

            // Ограничение вертикального поворота
            if (camY < -glm::radians(89.0f)) {
                camY = -glm::radians(89.0f);
            } else if (camY > glm::radians(89.0f)) {
                camY = glm::radians(89.0f);
            }

            // Применение поворота к камере
            camera->rotation = glm::mat4(1.0f);
            camera->rotate(camY, camX, 0);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Очистка буферов цвета и глубины

        shader->use(); // Активация шейдерной программы

        // Установка uniform-переменных для шейдера
        shader->uniformMatrix("model", model);
        shader->uniformMatrix("projview", camera->getProjection() * camera->getView());

        texture->bind(); // Привязка текстурного атласа

        mesh->draw(GL_TRIANGLES); // Отрисовка воксельного меша

        Window::swapBuffers(); // Обмен буферов
        Events::pollEvents(); // Обработка событий
    }

    // Очищаем ресурсы
    delete chunk;
    delete mesh;
    delete texture;
    delete shader;

    // Завершение работы
    Window::terminate();
    return 0;
}
