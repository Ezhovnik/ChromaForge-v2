#ifndef GRAPHICS_BATCH2D_H_
#define GRAPHICS_BATCH2D_H_

#include <stdlib.h>
#include <glm/glm.hpp>
#include <memory>

#include "../typedefs.h"

class Mesh;
class Texture;
class UVRegion;

/**
 * @brief Класс для пакетной отрисовки 2D-примитивов (спрайтов, прямоугольников, линий).
 *
 * Накапливает вершины во внутреннем буфере и отправляет их в OpenGL.
 * Поддерживает текстурирование, изменение цвета,
 * повороты, отражения и закруглённые прямоугольники.
 */
class Batch2D {
	float* buffer;
	size_t capacity;

	std::unique_ptr<Mesh> mesh;
	size_t index;

	glm::vec4 color;

	std::unique_ptr<Texture> blank; ///< Белая текстура 1x1 для случаев, когда текстура не задана
	Texture* _texture; ///< Текущая активная текстура

	/**
     * @brief Добавляет одну вершину в буфер.
     * @param x,y Координаты вершины.
     * @param u,v Текстурные координаты.
     * @param r,g,b,a Цвет (RGBA).
     */
	void vertex(
		float x, float y, 
		float u, float v, float r, 
		float g, float b, float a
	);

	/**
     * @brief Добавляет одну вершину в буфер (перегрузка с glm::vec2).
     * @param point Позиция.
     * @param uvpoint Текстурные координаты.
     * @param r,g,b,a Цвет.
     */
	void vertex(
		glm::vec2 point, 
		glm::vec2 uvpoint, 
		float r, float g, float b, float a
	);
public:
	/**
     * @brief Конструктор.
     * @param capacity Количество вершин, которое может быть сохранено до принудительного flush.
     */
	Batch2D(size_t capacity);

	~Batch2D();

	/**
     * @brief Начинает новый кадр/пакет. Устанавливает цвет по умолчанию и биндит пустую текстуру.
     */
	void begin();

	/**
     * @brief Устанавливает текущую текстуру. Если текстура отличается от предыдущей,
     *        вызывает flush() для отрисовки накопленных данных.
     * @param new_texture Указатель на текстуру (может быть nullptr).
     */
	void texture(Texture* texture);

	/**
     * @brief Сбрасывает текстуру в nullptr (биндит blank).
     */
	void untexture();

	/**
     * @brief Рисует прямоугольник с текстурной областью.
     * @param x,y Позиция левого верхнего угла.
     * @param w,h Размеры.
     * @param region Текстурная область.
     * @param tint Цветовой оттенок.
     */
	void sprite(float x, float y, float w, float h, const UVRegion& region, glm::vec4 tint);

	/**
     * @brief Рисует спрайт из атласа по индексу.
     * @param x,y Позиция.
     * @param w,h Размеры.
     * @param atlasRes Разрешение атласа (количество ячеек по одной стороне).
     * @param index Индекс ячейки (слева направо, сверху вниз).
     * @param tint Цвет.
     */
	void sprite(float x, float y, float w, float h, int atlasRes, int index, glm::vec4 tint);

	/**
     * @brief Рисует точку.
     * @param x,y Координаты.
     * @param r,g,b,a Цвет.
     */
     void point(float x, float y, float r, float g, float b, float a);

	/**
     * @brief Рисует линию.
     * @param x1,y1 Начальная точка.
     * @param x2,y2 Конечная точка.
     * @param r,g,b,a Цвет.
     */
	void line(float x1, float y1, float x2, float y2, float r, float g, float b, float a);

	/**
     * @brief Устанавливает текущий цвет по умолчанию.
     * @param color Цвет (RGBA).
     */
	inline void setColor(glm::vec4 color) {
          this->color = color;
     }

	/**
     * @brief Возвращает текущий цвет по умолчанию.
     */
     inline glm::vec4 getColor() const {
          return color;
     }

	/**
     * @brief Рисует прямоугольник с текстурой, поворотом и отражением.
     * @param x,y Позиция центра вращения? (на самом деле левый верхний угол после поворота? Сложно).
     * @param w,h Размеры.
     * @param ox,oy Центр вращения относительно размеров (0..1).
     * @param angle Угол поворота в радианах.
     * @param region Текстурная область.
     * @param flippedX,flippedY Отражение по горизонтали/вертикали.
     * @param tint Цвет.
     */
	void rect(
		float x, float y, 
		float w, float h, 
		float ox, float oy, 
		float angle, 
		UVRegion region, 
		bool flippedX, bool flippedY, 
		glm::vec4 tint
	);

	/**
     * @brief Рисует прямоугольник с текущим цветом и без текстуры (используется blank).
     * @param x,y Позиция.
     * @param w,h Размеры.
     */
	void rect(float x, float y, float w, float h);

	/**
     * @brief Рисует прямоугольник с текстурой, заданной явными координатами.
     * @param x,y Позиция.
     * @param w,h Размеры.
     * @param u,v Начальные текстурные координаты.
     * @param tx,ty Размер текстурной области (ширина, высота).
     * @param r,g,b,a Цвет.
     */
	void rect(
		float x, float y, 
		float w, float h, 
		float u, float v, 
		float tx, float ty, 
		float r, float g, float b, float a
	);
     void rect(
		float x, float y, 
		float w, float h, 
		float r0, float g0, float b0, 
		float r1, float g1, float b1, 
		float r2, float g2, float b2, 
		float r3, float g3, float b3, 
		float r4, float g4, float b4, 
		int sh
	);

	/**
     * @brief Принудительно отправляет накопленные вершины в OpenGL с указанным типом примитива.
     * @param gl_primitive Тип примитива OpenGL (например, GL_TRIANGLES, GL_LINES).
     */
     void flush(uint gl_primitive);

	/**
     * @brief Отправляет накопленные вершины как треугольники (GL_TRIANGLES).
     */
     void flush();

	/**
     * @brief Устанавливает толщину линии (через glLineWidth).
     * @param width Толщина в пикселях.
     */
     void setLineWidth(float width);
};


#endif // GRAPHICS_BATCH2D_H_
