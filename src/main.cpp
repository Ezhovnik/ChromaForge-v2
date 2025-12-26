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
#include "voxels/Chunks.h"
#include "graphics/VoxelRenderer.h"
#include "graphics/LineBatch.h"
#include "files/files.h"

// Размеры окна по умолчанию
int WIDTH = 1280;
int HEIGHT = 720;

// Размеры набора чанков
constexpr int CHUNKS_X = 16;
constexpr int CHUNKS_Y = 8;
constexpr int CHUNKS_Z = 16;

constexpr float MAX_PITCH = glm::radians(89.0f); // Ограничесние вертикального поворота

// Вершины прицела-указателя
const float crosshair_vertices[] = {
    -0.01f, -0.01f,
    0.01f, 0.01f,

    -0.01f, 0.01f,
    0.01f, -0.01f,
};

// Атрибуты вершин прицела-указателя
const int crosshair_attrs[] = {
    2, 0
};

int main() {
    Window::initialize(WIDTH, HEIGHT, "ChromaForge"); // Инициализация окна
    Events::initialize(); // Инициализация системы событий

    // Загрузка шейдерной программы
    ShaderProgram* shader = loadShaderProgram("../res/shaders/default.vert", "../res/shaders/default.frag");
    if (shader == nullptr) {
        std::cerr << "Failed to load shader program" << std::endl;
        Window::terminate();
        return -1;
    }

    // Загрузка шейдерной программы прицела-указателя
    ShaderProgram* crosshair_shader = loadShaderProgram("../res/shaders/crosshair.vert", "../res/shaders/crosshair.frag");
    if (crosshair_shader == nullptr) {
        std::cerr << "Failed to load crosshair shader program" << std::endl;
        delete shader;
        Window::terminate();
        return -1;
    }

    // Загрузка шейдерной программы для отрисовки линий
    ShaderProgram* lines_shader = loadShaderProgram("../res/shaders/lines.vert", "../res/shaders/lines.frag");
    if (lines_shader == nullptr) {
        std::cerr << "Failed to load lines shader program" << std::endl;
        delete crosshair_shader;
        delete shader;
        Window::terminate();
        return -1;
    }

    // Загрузка текстурного атласа
    Texture* texture = loadTexture("../res/textures/atlas.png");
    if (texture == nullptr) {
        std::cerr << "Failed to load texture" << std::endl;
        delete lines_shader;
        delete crosshair_shader;
        delete shader;
        Window::terminate();
        return -1;
    }

    Chunks* chunks = new Chunks(CHUNKS_X, CHUNKS_Y, CHUNKS_Z); // Создание и инициализация мира из чанков
    Mesh** meshes = new Mesh*[chunks->volume]; // Массив мешей для каждого чанка
    for (size_t i = 0; i < chunks->volume; ++i) {
        meshes[i] = nullptr;
    }
    VoxelRenderer renderer(1024 * 1024 * 8); // Создание рендерера вокселей с заданной емкостью буфера
    LineBatch* lineBatch = new LineBatch(4096); // Буфер для пакетной отрисовки линий

    glClearColor(0.6f, 0.62f, 0.65f, 1.0f); // Серый цвет фона

    glEnable(GL_DEPTH_TEST); // Включение теста глубины

    // Включение отсечения задних граней для оптимизации
    glEnable(GL_CULL_FACE); 
    glCullFace(GL_BACK);
    
    glEnable(GL_BLEND); // Включение смешивания цветов
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Режим смешивания для прозрачности

    Mesh* crosshair_mesh = new Mesh(crosshair_vertices, 4, crosshair_attrs); // Создание меша для прицела-указателя

    Camera* camera = new Camera(glm::vec3(0, 0, 1), glm::radians(70.0f)); // Создание камеры

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

        // Клавиши для сохранения и загрузки мира
        if (Events::justPressed(GLFW_KEY_F1)) {
            // Сохраняем мир (F1)
            unsigned char* buffer = new unsigned char[chunks->volume * CHUNK_VOLUME];
            chunks->write(buffer);
            if (!write_binary_file("../saves/world.bin", (const char*)buffer, chunks->volume * CHUNK_VOLUME)) {
                std::cerr << "Error: Failed to save world to file" << std::endl;
            } else {
                std::cout << "World saved in " << (chunks->volume * CHUNK_VOLUME) << " bytes" << std::endl;
            }
            delete[] buffer;
        }
        if (Events::justPressed(GLFW_KEY_F2)) {
            // Загружаем мир (F2)
            unsigned char* buffer = new unsigned char[chunks->volume * CHUNK_VOLUME];
            if (!read_binary_file("../saves/world.bin", (char*)buffer, chunks->volume * CHUNK_VOLUME)) {
                std::cerr << "Error: Failed to read world from file" << std::endl;
            } else {
                chunks->read(buffer);
                std::cout << "World loaded successfully" << std::endl;
            }
            delete[] buffer;
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

        // Вертикальное движение камеры
        if (Events::isPressed(GLFW_KEY_SPACE)) {
            camera->position += camera->up * deltaTime * speed; // Вверх (пробел)
        }
        if (Events::isPressed(GLFW_KEY_LEFT_SHIFT)) {
            camera->position -= camera->up * deltaTime * speed; // Вниз (Shift)
        }

        // Управление поворотом камеры мышью (только в заблокированном режиме)
        if (Events::_cursor_locked) {
            // Обновление углов поворота на основе движения мыши
            camY -= Events::deltaY / Window::width; // Вертикальный поворот
            camX -= Events::deltaX / Window::height; // Горизонтальный поворот

            // Ограничение вертикального поворота
            if (camY < -MAX_PITCH) {
                camY = -MAX_PITCH;
            } else if (camY > MAX_PITCH) {
                camY = MAX_PITCH;
            }

            // Применение поворота к камере
            camera->rotation = glm::mat4(1.0f);
            camera->rotate(camY, camX, 0);
        }

        // Логика взаимодействия с вокселями (разрушение/установка блоков)
        {
            glm::vec3 hitPoint; // Точка попадания луча
            glm::vec3 hitNormal; // Нормаль поверхности в точке попадания
            glm::vec3 hitVoxelCoord; // Координаты вокселя в точке попадания
            voxel* vox = chunks->rayCast(camera->position, camera->front, 10.0f, hitPoint, hitNormal, hitVoxelCoord);
            if (vox != nullptr) {
                // Рисуем обводку для блока, на который смотрит камера
                lineBatch->box(
                    hitVoxelCoord.x + 0.5f, hitVoxelCoord.y + 0.5f, hitVoxelCoord.z + 0.5f,
                    1.01f, 1.01f, 1.01f,
                    0, 0, 0, 1
                );

                // Ломаем блок на ЛКМ
                if (Events::justClicked(GLFW_MOUSE_BUTTON_1)) {
                    chunks->setVoxel((int)hitVoxelCoord.x, (int)hitVoxelCoord.y, (int)hitVoxelCoord.z, 0);
                }
                // Ставим землю (id = 2) на ПКМ
                if (Events::justClicked(GLFW_MOUSE_BUTTON_2)) {
                    chunks->setVoxel((int)hitVoxelCoord.x + (int)hitNormal.x, (int)hitVoxelCoord.y + (int)hitNormal.y, (int)hitVoxelCoord.z + (int)hitNormal.z, 2);
                }
            }
        }

        // Обновление мешей чанков, требующих перестроения
        Chunk* neighborChunks[27];
        for (size_t i = 0; i < chunks->volume; ++i) {
            Chunk* chunk = chunks->chunks[i];

            // Пропускаем чанки, не требующие обновления
            if (!chunk->needsUpdate){
                continue;
            }
            chunk->needsUpdate = false;

            // Освобождаем старый меш, если он существует
            if (meshes[i] != nullptr) {
                delete meshes[i];
                meshes[i] = nullptr;
            }

            // Собираем информацию о соседних чанках
            for (int j = 0; j < 27; ++j) {
                neighborChunks[j] = nullptr;
            }

            for (size_t j = 0; j < chunks->volume; ++j) {
                Chunk* other = chunks->chunks[j];

                // Вычисляем относительные координаты
                int dx = other->chunk_x - chunk->chunk_x;
                int dy = other->chunk_y - chunk->chunk_y;
                int dz = other->chunk_z - chunk->chunk_z;

                // Пропускаем чанки, которые не являются непосредственными соседями
                if (abs(dx) > 1 || abs(dy) > 1 || abs(dz) > 1) {
                    continue;
                }

                neighborChunks[((dy + 1) * 3 + dz + 1) * 3 + dx + 1] = other;
            }

            // Генерация нового меша для чанка
            Mesh* mesh = renderer.render(chunk, (const Chunk**) neighborChunks, true);
            meshes[i] = mesh;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Очистка буферов цвета и глубины

        // Активация шейдерной программы
        shader->use();

        // Установка uniform-переменных для шейдера
        shader->uniformMatrix("projview", camera->getProjection() * camera->getView());

        // Привязка текстурного атласа
        texture->bind();

        // Рендеринг всех чанков
        glm::mat4 model;
        for (size_t i = 0; i < chunks->volume; ++i) {
            Chunk* chunk = chunks->chunks[i];
            Mesh* mesh = meshes[i];

            // Создание матрицы модели для текущего чанка
            model = glm::translate(
                glm::mat4(1.0f),
                glm::vec3(
                    chunk->chunk_x * CHUNK_WIDTH + 0.5f,
                    chunk->chunk_y * CHUNK_HEIGHT + 0.5f,
                    chunk->chunk_z * CHUNK_DEPTH + 0.5f
                )
            );

            // Передача матрицы модели в шейдер
            shader->uniformMatrix("model", model);

            // Отрисовка меша чанка
            mesh->draw(GL_TRIANGLES);
        }

        // Рендеринг прицела-указателя
        crosshair_shader->use();
        crosshair_mesh->draw(GL_LINES);

        // Рендеринг обводки
        lines_shader->use();
        lines_shader->uniformMatrix("projview", camera->getProjection() * camera->getView());
        glLineWidth(2.0f); // Толщина обводки
        lineBatch->render();

        Window::swapBuffers(); // Обмен буферов
        Events::pollEvents(); // Обработка событий
    }

    // Очищаем ресурсы
    delete crosshair_mesh;
    delete lineBatch;

    for (size_t i = 0; i < chunks->volume; ++i) {
        if (meshes[i] != nullptr) {
            delete meshes[i];
        }
    }
    delete[] meshes;

    delete chunks;
    delete texture;

    delete lines_shader;
    delete crosshair_shader;
    delete shader;

    // Завершение работы
    Window::terminate();
    return 0;
}
