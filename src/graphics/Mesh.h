#ifndef GRAPHICS_MESH_H_
#define GRAPHICS_MESH_H_

#include "../typedefs.h"

// Класс графической сетки (меша)
class Mesh {
    uint VAO; // Идентификатор VAO (массив вершин)
    uint VBO; // Идентификатор VBO (буфер вершин)
    size_t vertices; // Количество вершин в меше
    size_t vertexSize; // Размер вершины
public:
    Mesh (const float* buffer, size_t vertices, const int* attrs); // Конструктор
    ~Mesh(); // Деструктор

    void reload(const float* buffer, size_t vertices); // Обновляет данные вершин меша
    void draw(uint primititve); // Отрисовывает меш с указанным типом примитива.
};

#endif // GRAPHICS_MESH_H_
