#ifndef GRAPHICS_CORE_MESH_H_
#define GRAPHICS_CORE_MESH_H_

#include "../../typedefs.h"

/**
 * @brief Описание одного вершинного атрибута.
 *
 * Используется для задания количества компонентов в атрибуте (например, 3 для позиции, 2 для текстурных координат).
 * Массив таких структур должен заканчиваться элементом с size = 0.
 */
struct vattr {
    ubyte size; ///< Количество компонентов атрибута
};

/**
 * @brief Класс для работы с графической сеткой (мешем) в OpenGL.
 *
 * Управляет буферами вершин (VBO), индексов (IBO) и объектом вершинного массива (VAO).
 * Поддерживает статические меши (данные загружаются один раз) и их перезагрузку.
 */
class Mesh {
    uint VAO; ///< Идентификатор объекта вершинного массива (Vertex Array Object)
    uint VBO; ///< Идентификатор буфера вершин (Vertex Buffer Object)
    uint IBO; ///< Идентификатор буфера индексов (Index Buffer Object), 0 если не используется
    size_t vertices; ///< Количество вершин в меше
    size_t indices; ///< Количество индексов (если используется индексный буфер)
    size_t vertexSize; ///< Размер одной вершины
public:
    /**
     * @brief Конструктор, создающий меш из данных вершин и индексов.
     * @param vertexBuffer Указатель на массив вершин (float).
     * @param vertices Количество вершин.
     * @param indexBuffer Указатель на массив индексов (int), может быть nullptr.
     * @param indices Количество индексов (0 если индексный буфер не используется).
     * @param attrs Массив описаний вершинных атрибутов, заканчивающийся элементом с size = 0.
     */
    Mesh(
        const float* vertexBuffer, 
        size_t vertices, 
        const int* indexBuffer, 
        size_t indices, 
        const vattr* attrs
    );

    /**
     * @brief Конструктор для создания меша без индексного буфера.
     * @param vertexBuffer Указатель на массив вершин.
     * @param vertices Количество вершин.
     * @param attrs Описания атрибутов.
     */
	Mesh(const float* vertexBuffer, size_t vertices, const vattr* attrs) :
		Mesh(vertexBuffer, vertices, nullptr, 0, attrs) {};

    /// Деструктор, освобождающий ресурсы OpenGL.
    ~Mesh();

    /**
     * @brief Перезагружает данные меша (заменяет буферы новыми данными).
     * @param vertexBuffer Новый массив вершин.
     * @param vertices Новое количество вершин.
     * @param indexBuffer Новый массив индексов (может быть nullptr).
     * @param indices Новое количество индексов.
     *
     * Размер вершины (vertexSize) остаётся тем же, что был задан в конструкторе.
     * Если индексный буфер не использовался ранее и передаётся nullptr, он остаётся неактивным.
     * Если индексный буфер был и передаётся nullptr, старый IBO удаляется.
     */
    void reload(
        const float* vertexBuffer, 
        size_t vertices, 
        const int* indexBuffer = nullptr, 
        size_t indices = 0
    );

    /**
     * @brief Отрисовывает меш с указанным типом примитива.
     * @param primitive Тип примитива OpenGL (например, GL_TRIANGLES, GL_LINES).
     */
    void draw(uint primititve);

    /**
     * @brief Отрисовывает меш с типом примитива GL_TRIANGLES.
     */
    void draw();

    /// Счётчик всех созданных экземпляров Mesh (для отладки).
    static int meshesCount;
};

#endif // GRAPHICS_CORE_MESH_H_
