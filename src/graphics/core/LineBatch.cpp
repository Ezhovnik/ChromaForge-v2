#include <graphics/core/LineBatch.h>

#include <GL/glew.h>

#include <graphics/core/Mesh.h>

// Конструктор
LineBatch::LineBatch(size_t capacity) : capacity(capacity) {
    buffer = std::make_unique<LineVertex[]>(capacity * 2);
    mesh = std::make_unique<Mesh<LineVertex>>(buffer.get(), 0);
    index = 0;
}

// Деструктор
LineBatch::~LineBatch() {
}

// Добавляет линию в буфер для отрисовки
void LineBatch::line(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, float red, float green, float blue, float alpha) {
    if (index + 2 >= capacity) flush();

    // Записываем данные начальной вершины
    buffer[index].position = {start_x, start_y, start_z};
    buffer[index].color = {red, green, blue, alpha};
    index++;

    // Записываем данные конечной вершины
    buffer[index].position = {end_x, end_y, end_z};
    buffer[index].color = {red, green, blue, alpha};
    index++;
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
void LineBatch::flush() {
    if (index == 0) return;

    mesh->reload(buffer.get(), index);
    mesh->draw(GL_LINES);
    index = 0;
}

void LineBatch::setLineWidth(float width) {
    glLineWidth(width);
}
