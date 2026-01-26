#include "Mesh.h"

#include <GL/glew.h>

int Mesh::meshesCount = 0;

// Конструктор
Mesh::Mesh(const float* vertexBuffer, size_t vertices, const int* indexBuffer, size_t indices, const vattr* attrs) :
    vertices(vertices),
    indices(indices),
    IBO(0) 
{
    meshesCount++;

    // Вычисление размера вершины
    vertexSize = 0;
    for (int i = 0; attrs[i].size; ++i) {
        vertexSize += attrs[i].size;
    }

    // Создание объектов OpenGL
    glGenVertexArrays(1, &VAO); // Создание VAO
    glGenBuffers(1, &VBO); // Создание VBO

    reload(vertexBuffer, vertices, indexBuffer, indices);

    // Настройка вершинных атрибутов
    int offset = 0; // Смещение в байтах от начала вершины
    for (int i = 0; attrs[i].size; ++i) {
        int size = attrs[i].size; // Количество компонентов атрибута
        glVertexAttribPointer(
            i, // Индекс атрибута (location в шейдере)
            size, // Количество компонентов
            GL_FLOAT, // Тип данных
            GL_FALSE, 
            vertexSize * sizeof(float), // Шаг между вершинами (в байтах)
            (GLvoid*)(offset * sizeof(float))
        ); // Указание формата атрибута
        glEnableVertexAttribArray(i); // Включение атрибута с индексом i
        offset += size;
    }

    glBindVertexArray(0); // Отвязываем VAO
}

// Деструктор
Mesh::~Mesh() {
    meshesCount--;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    if (IBO != 0) glDeleteBuffers(1, &IBO);
}

// Обновляет данные вершин меша
void Mesh::reload(const float* vertexBuffer, size_t vertices, const int* indexBuffer, size_t indices) {
    glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	
    if (vertexBuffer != nullptr && vertices != 0) {
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexSize * vertices, vertexBuffer, GL_STATIC_DRAW);
    } else {
        glBufferData(GL_ARRAY_BUFFER, 0, {}, GL_STATIC_DRAW);
    }

    if (indexBuffer != nullptr && indices != 0) {
        if (IBO == 0) glGenBuffers(1, &IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices, indexBuffer, GL_STATIC_DRAW);
    } else if (IBO != 0) {
        glDeleteBuffers(1, &IBO);
    }

    this->vertices = vertices;
    this->indices = indices;
}

// Отрисовывает меш с использованием указанного типа примитива.
void Mesh::draw(uint primitive) {
    glBindVertexArray(VAO);
    if (IBO != 0) {
        glDrawElements(primitive, indices, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(primitive, 0, vertices);
    }
    glBindVertexArray(0);
}

void Mesh::draw() {
    draw(GL_TRIANGLES);
}
