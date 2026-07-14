#pragma once

#include <vector>

#include <graphics/core/MeshData.h>

struct MeshStats {
    static int meshesCount;
    static int drawCalls;
};

struct IndexBufferData {
    const uint32_t* indices;
    size_t indicesCount;
};

template<typename VertexStructure>
class Mesh {
    struct IndexBuffer {
        unsigned int ibo;
        size_t indexCount;
    };

    uint VAO; ///< Идентификатор объекта вершинного массива (Vertex Array Object)
    uint VBO; ///< Идентификатор буфера вершин (Vertex Buffer Object)
    std::vector<IndexBuffer> IBOs;
    size_t vertexCount;
public:
    explicit Mesh(const MeshData<VertexStructure>& data);

    Mesh(
        const VertexStructure* vertexBuffer,
        size_t vertices,
        std::vector<IndexBufferData> indices
    );

    Mesh(
        const VertexStructure* vertexBuffer, size_t vertices
    ) : Mesh<VertexStructure>(vertexBuffer, vertices, {}) {};

    /// Деструктор, освобождающий ресурсы OpenGL.
    ~Mesh();

    void reload(
        const VertexStructure* vertexBuffer,
        size_t vertexCount,
        const std::vector<IndexBufferData>& indices
    );

    void reload(const VertexStructure* vertexBuffer, size_t vertexCount) {
        static const std::vector<IndexBufferData> indices {};
        reload(vertexBuffer, vertexCount, indices);
    }

    /**
     * @brief Отрисовывает меш с указанным типом примитива.
     * @param primitive Тип примитива OpenGL (например, GL_TRIANGLES, GL_LINES).
     */
    void draw(unsigned int primitive, int iboIndex = 0) const;

    /**
     * @brief Отрисовывает меш с типом примитива GL_TRIANGLES.
     */
    void draw() const;
};

#include <graphics/core/Mesh.inl>
