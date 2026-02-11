#ifndef GRAPHICS_MESH_H_
#define GRAPHICS_MESH_H_

#include "../typedefs.h"

struct vattr {
    ubyte size;
};

// Класс графической сетки (меша)
class Mesh {
    uint VAO; // Идентификатор VAO (массив вершин)
    uint VBO; // Идентификатор VBO (буфер вершин)
    uint IBO;
    size_t vertices; // Количество вершин в меше
    size_t indices;
    size_t vertexSize; // Размер вершины
public:
    Mesh(const float* vertexBuffer, size_t vertices, const int* indexBuffer, size_t indices, const vattr* attrs);
	Mesh(const float* vertexBuffer, size_t vertices, const vattr* attrs) :
		Mesh(vertexBuffer, vertices, nullptr, 0, attrs) {};
    ~Mesh(); // Деструктор

    void reload(const float* vertexBuffer, size_t vertices, const int* indexBuffer = nullptr, size_t indices = 0);
    void draw(uint primititve); // Отрисовывает меш с указанным типом примитива.
    void draw();

    static int meshesCount;
};

#endif // GRAPHICS_MESH_H_
