#include <graphics/core/Mesh.h>

#include <assert.h>

#include <GL/glew.h>

int Mesh::meshesCount = 0;
int Mesh::drawCalls = 0;

inline size_t calc_vertex_size(const VertexAttribute* attrs) {
    size_t vertexSize = 0;
    for (int i = 0; attrs[i].size; ++i) {
        vertexSize += attrs[i].size;
    }
    assert(vertexSize != 0);
    return vertexSize;
}

Mesh::Mesh(
    const MeshData& data
) : Mesh(
        data.vertices.data(),
        data.vertices.size() / calc_vertex_size(data.attrs.data()),
        data.indices.data(),
        data.indices.size(),
        data.attrs.data()
    ) {}

Mesh::Mesh(const float* vertexBuffer, 
    size_t vertices, 
    const int* indexBuffer, 
    size_t indices, 
    const VertexAttribute* attrs
) : IBO(0),
    vertices(0),
	indices(0)
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
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexSize * vertices, vertexBuffer, GL_STREAM_DRAW);
	} else {
		glBufferData(GL_ARRAY_BUFFER, 0, {}, GL_STREAM_DRAW);
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

void Mesh::draw(uint primitive) const {
    drawCalls++;
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

void Mesh::draw() const {
    draw(GL_TRIANGLES);
}
