#include "Mesh.h"

#include <GL/glew.h>

int Mesh::meshesCount = 0;

Mesh::Mesh(const float* vertexBuffer, 
    size_t vertices, 
    const int* indexBuffer, 
    size_t indices, 
    const vattr* attrs
) : IBO(0),
    vertices(vertices),
	indices(indices)
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
            (void*)(offset * sizeof(float))
        ); // Указание формата атрибута
        glEnableVertexAttribArray(i); // Включение атрибута с индексом i
        offset += size;
    }

    glBindVertexArray(0); // Отвязываем VAO
}

Mesh::~Mesh() {
    meshesCount--;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    if (IBO != 0) glDeleteBuffers(1, &IBO); // удаляем индексный буфер, если он был создан
}

void Mesh::reload(const float* vertexBuffer, size_t vertices, const int* indexBuffer, size_t indices){
    this->vertices = vertices;
    this->indices = indices;

    glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Загружаем данные вершин в VBO.
	if (vertexBuffer != nullptr && vertices != 0) {
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexSize * vertices, vertexBuffer, GL_STATIC_DRAW);
	} else {
		glBufferData(GL_ARRAY_BUFFER, 0, {}, GL_STATIC_DRAW);
	}

    // Обрабатываем индексный буфер.
	if (indexBuffer != nullptr && indices != 0) {
		if (IBO == 0) glGenBuffers(1, &IBO); // создаём IBO, если его ещё нет
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices, indexBuffer, GL_STATIC_DRAW);
	} else if (IBO != 0) {
        // Если раньше использовался индексный буфер, а теперь он не нужен — удаляем.
		glDeleteBuffers(1, &IBO);
	}
}

void Mesh::draw(uint primitive) {
    glBindVertexArray(VAO);
    if (IBO != 0) {
        // Рисование с использованием индексного буфера.
		glDrawElements(primitive, indices, GL_UNSIGNED_INT, 0);
	} else {
        // Рисование без индексов (просто массивы вершин).
		glDrawArrays(primitive, 0, vertices);
	}
    glBindVertexArray(0);
}

void Mesh::draw() {
    draw(GL_TRIANGLES);
}
