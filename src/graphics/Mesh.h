#ifndef GRAPHICS_MESH_H_
#define GRAPHICS_MESH_H_

#include <stdlib.h>

typedef unsigned int uint;

// Класс графической сетки (меша)
class Mesh {
    uint VAO; // Идентификатор VAO (массив вершин)
    uint VBO; // Идентификатор VBO (буфер вершин)
    size_t vertices; // Количество вершин в меше
public:
    Mesh (const float* buffer, size_t vertices, const int* attrs); // Конструктор
    ~Mesh(); // Деструктор

    void draw(uint primititve); // Отрисовывает меш с указанным типом примитива.
};

#endif // GRAPHICS_MESH_H_
