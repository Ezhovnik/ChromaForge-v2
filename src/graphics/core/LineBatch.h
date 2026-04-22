#pragma once

#include <stdlib.h>
#include <memory>

#include <glm/glm.hpp>

#include <graphics/core/commons.h>

class Mesh;

// Буфер для пакетной отрисовки линий
class LineBatch : public Flushable {
private:
    std::unique_ptr<float[]> buffer; // Указатель на буфер вершинных данных
    size_t index; // Текущая позиция в буфере (в вершинах)
    size_t capacity;  // Максимальная вместимость буфера (в вершинах)

    std::unique_ptr<Mesh> mesh; // Указатель на Mesh объект для рендеринга
public:
    LineBatch(size_t capacity = 4096); // Конструктор
    ~LineBatch(); // Деструктор

    void line( // Добавляет линию в буфер для отрисовки
        float start_x, float start_y, float start_z,
        float end_x, float end_y, float end_z,
        float red, float green, float blue, float alpha
    );
    inline void line(const glm::vec3 a, const glm::vec3 b, const glm::vec4 color) {
		line(a.x, a.y, a.z, b.x, b.y, b.z, color.r, color.g, color.b, color.a); 
	}

    void box( // Добавляет прямоугольный параллелепипед (бокс) в буфер для отрисовки
        float x, float y, float z, 
        float width, float height, float depth, 
        float red, float green, float blue, float alpha
    );
    inline void box(glm::vec3 xyz, glm::vec3 whd, glm::vec4 rgba) {
		box(xyz.x, xyz.y, xyz.z, whd.x, whd.y, whd.z, rgba.r, rgba.g, rgba.b, rgba.a);
	}

    void flush() override;

    void setLineWidth(float width);
};
