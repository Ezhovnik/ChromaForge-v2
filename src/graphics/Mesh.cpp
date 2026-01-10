#include "Mesh.h"

#include <GL/glew.h>

#include "../logger/OpenGL_Logger.h"

// Конструктор
Mesh::Mesh(const float* buffer, size_t vertices, const int* attrs) : vertices(vertices) {
    // Вычисление размера вершины
    vertexSize = 0;
    for (int i = 0; attrs[i]; ++i) {
        vertexSize += attrs[i];
    }

    // Создание объектов OpenGL
    glGenVertexArrays(1, &VAO); // Создание VAO
    glGenBuffers(1, &VBO); // Создание VBO

    glBindVertexArray(VAO); // Привязка VAO
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Привязка VBO
    if (buffer){
		glBufferData(
            GL_ARRAY_BUFFER, 
            sizeof(float) * vertexSize * vertices, // Общий размер данных
            buffer, // Указатель на данные
            GL_STATIC_DRAW // GL_STATIC_DRAW указывает, что данные не будут изменяться часто
        ); // Загрузка вершин в буфер OpenGL
	} else {
		glBufferData(GL_ARRAY_BUFFER, 0, {}, GL_STATIC_DRAW);
	}
    

    // Настройка вершинных атрибутов
    int offset = 0; // Смещение в байтах от начала вершины
    for (int i = 0; attrs[i]; ++i) {
        int size = attrs[i]; // Количество компонентов атрибута
        glVertexAttribPointer(
            i, // Индекс атрибута (location в шейдере)
            size, // Количество компонентов
            GL_FLOAT, // Тип данных
            GL_FALSE, 
            vertexSize * sizeof(float), // Шаг между вершинами (в байтах)
            (void*)(offset * sizeof(float))
        ); // Указание формата атрибута
        glEnableVertexAttribArray(i); // Включение атрибута с индексом i
        offset += size;
    }

    glBindVertexArray(0); // Отвязываем VAO

    GL_CHECK();
}

// Деструктор
Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

// Обновляет данные вершин меша
void Mesh::reload(const float* buffer, size_t vertices) {
    this->vertices = vertices;

    glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexSize * vertices, buffer, GL_STATIC_DRAW);

    GL_CHECK();
}

// Отрисовывает меш с использованием указанного типа примитива.
void Mesh::draw(uint primitive) {
    glBindVertexArray(VAO);
    glDrawArrays(primitive, 0, vertices);
    glBindVertexArray(0);

    GL_CHECK();
}
