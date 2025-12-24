#include "Mesh.h"

#include <GL/glew.h>

// Конструктор
Mesh::Mesh(const float* buffer, size_t vertices, const int* attrs) : vertices(vertices) {
    // Вычисление размера вершины
    int vertex_size = 0;
    for (int i = 0; attrs[i]; ++i) {
        vertex_size += attrs[i];
    }

    // Создание объектов OpenGL
    glGenVertexArrays(1, &VAO); // Создание VAO
    glGenBuffers(1, &VBO); // Создание VBO

    glBindVertexArray(VAO); // Привязка VAO
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Привязка VBO
    glBufferData(
        GL_ARRAY_BUFFER, 
        sizeof(float) * vertex_size * vertices, // Общий размер данных
        buffer, // Указатель на данные
        GL_STATIC_DRAW // GL_STATIC_DRAW указывает, что данные не будут изменяться часто
    ); // Загрузка вершин в буфер OpenGL

    // Настройка вершинных атрибутов
    int offset = 0; // Смещение в байтах от начала вершины
    for (int i = 0; attrs[i]; ++i) {
        int size = attrs[i]; // Количество компонентов атрибута
        glVertexAttribPointer(
            i, // Индекс атрибута (location в шейдере)
            size, // Количество компонентов
            GL_FLOAT, // Тип данных
            GL_FALSE, 
            vertex_size * sizeof(float), // Шаг между вершинами (в байтах)
            (void*)(offset * sizeof(float))
        ); // Указание формата атрибута
        glEnableVertexAttribArray(i); // Включение атрибута с индексом i
        offset += size;
    }

    glBindVertexArray(0); // Отвязываем VAO
}

// Деструктор
Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

// Отрисовывает меш с использованием указанного типа примитива.
void Mesh::draw(uint primitive) {
    glBindVertexArray(VAO);
    glDrawArrays(primitive, 0, vertices);
    glBindVertexArray(0);
}
