#ifndef GRAPHICS_LINEBATCH_H_
#define GRAPHICS_LINEBATCH_H_

#include <stdlib.h>

class Mesh;

// Буфер для пакетной отрисовки линий
class LineBatch {
    float* buffer; // Указатель на буфер вершинных данных
    size_t index; // Текущая позиция в буфере (в вершинах)
    size_t capacity;  // Максимальная вместимость буфера (в вершинах)

    Mesh* mesh; // Указатель на Mesh объект для рендеринга
public:
    LineBatch(size_t capacity); // Конструктор
    ~LineBatch(); // Деструктор

    void line( // Добавляет линию в буфер для отрисовки
        float start_x, float start_y, float start_z,
        float end_x, float end_y, float end_z,
        float red, float green, float blue, float alpha
    );
    void box( // Добавляет прямоугольный параллелепипед (бокс) в буфер для отрисовки
        float x, float y, float z, 
        float width, float height, float depth, 
        float red, float green, float blue, float alpha
    );

    void render(); // Выполняет отрисовку всех накопленных линий

    void setLineWidth(float width);
};

#endif // GRAPHICS_LINEBATCH_H_
