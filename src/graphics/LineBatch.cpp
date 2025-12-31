#include "LineBatch.h"

#include <GL/glew.h>

#include "Mesh.h"

// Константы для формата вершины
constexpr int LB_VERTEX_SIZE = (3 + 4);
const int LB_ATTRS[] = {3, 4, 0}; // Атрибуты: позиция(3), цвет(4)

// Конструктор
LineBatch::LineBatch(size_t capacity) : capacity(capacity) {
    buffer = new float[capacity * LB_VERTEX_SIZE * 2];
    mesh = new Mesh(buffer, 0, LB_ATTRS);
    index = 0;
}

// Деструктор
LineBatch::~LineBatch() {
    delete[] buffer;
    delete mesh;
}

// Добавляет линию в буфер для отрисовки
void LineBatch::line(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, float red, float green, float blue, float alpha) {
    if (index >= capacity * LB_VERTEX_SIZE * 2) return;

    // Записываем данные начальной вершины
    buffer[index++] = start_x;
    buffer[index++] = start_y;
    buffer[index++] = start_z;
    buffer[index++] = red;
    buffer[index++] = green;
    buffer[index++] = blue;
    buffer[index++] = alpha;

    // Записываем данные конечной вершины
    buffer[index++] = end_x;
    buffer[index++] = end_y;
    buffer[index++] = end_z;
    buffer[index++] = red;
    buffer[index++] = green;
    buffer[index++] = blue;
    buffer[index++] = alpha;
}

// Добавляет прямоугольный параллелепипед (бокс) в буфер для отрисовки
void LineBatch::box(float x, float y, float z, float width, float height, float depth, float red, float green, float blue, float alpha) {
    // Преобразуем полные размеры в половины размеров (радиусы)
    width *= 0.5f;
    height *= 0.5f;
    depth *= 0.5f;

    line(
        x - width, y - height, z - depth, 
        x + width, y - height, z - depth, 
        red, green, blue, alpha
    );
	line(
        x - width, y + height, z - depth, 
        x + width, y + height, z - depth, 
        red, green, blue, alpha
    );
	line(
        x - width, y - height, z + depth, 
        x + width, y - height, z + depth, 
        red, green, blue, alpha
    );
	line(x - width, y + height, z + depth, 
        x + width, y + height, z + depth, 
        red, green, blue, alpha
    );

	line(
        x - width, y - height, z - depth, 
        x - width, y + height, z - depth, 
        red, green, blue, alpha
    );
	line(
        x + width, y - height, z - depth, 
        x + width, y + height, z - depth, 
        red, green, blue, alpha
    );
	line(
        x - width, y - height, z + depth, 
        x - width, y + height, z + depth, 
        red, green, blue, alpha
    );
	line(
        x + width, y - height, z + depth, 
        x + width, y + height, z + depth, 
        red, green, blue, alpha
    );

	line(
        x - width, y - height, z - depth, 
        x - width, y - height, z + depth, 
        red, green, blue, alpha
    );
	line(
        x + width, y - height, z - depth, 
        x + width, y - height, z + depth, 
        red, green, blue, alpha
    );
	line(
        x - width, y + height, z - depth, 
        x - width, y + height, z + depth, 
        red, green, blue, alpha
    );
	line(
        x + width, y + height, z - depth, 
        x + width, y + height, z + depth, 
        red, green, blue, alpha
    );
}

// Выполняет отрисовку всех накопленных линий
void LineBatch::render() {
    if (index == 0) return;

    mesh->reload(buffer, index / LB_VERTEX_SIZE);
    mesh->draw(GL_LINES);
    index = 0;
}
