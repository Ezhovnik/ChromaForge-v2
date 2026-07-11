#pragma once

#include <graphics/core/MeshData.h>

struct MeshStats {
    static int meshesCount;
    static int drawCalls;
};

template<typename VertexStructure>
class Mesh {
    uint VAO; ///< Идентификатор объекта вершинного массива (Vertex Array Object)
    uint VBO; ///< Идентификатор буфера вершин (Vertex Buffer Object)
    uint IBO; ///< Идентификатор буфера индексов (Index Buffer Object), 0 если не используется
    size_t vertexCount;
    size_t indexCount;
    size_t vertexSize; ///< Размер одной вершины
public:
    explicit Mesh(const MeshData<VertexStructure>& data);

    Mesh(
        const VertexStructure* vertexBuffer,
        size_t vertices,
        const uint32_t* indexBuffer,
        size_t indices
    );

    Mesh(
        const VertexStructure* vertexBuffer, size_t vertices
    ) : Mesh<VertexStructure>(vertexBuffer, vertices, nullptr, 0) {};

    /// Деструктор, освобождающий ресурсы OpenGL.
    ~Mesh();

    void reload(
        const VertexStructure* vertexBuffer,
        size_t vertexCount,
        const uint32_t* indexBuffer = nullptr,
        size_t indexCount = 0
    );

    /**
     * @brief Отрисовывает меш с указанным типом примитива.
     * @param primitive Тип примитива OpenGL (например, GL_TRIANGLES, GL_LINES).
     */
    void draw(uint primititve) const;

    /**
     * @brief Отрисовывает меш с типом примитива GL_TRIANGLES.
     */
    void draw() const;
};

#include <graphics/core/Mesh.inl>
