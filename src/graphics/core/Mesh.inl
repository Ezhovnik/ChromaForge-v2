#pragma once

#include <graphics/core/MeshData.h>
#include <graphics/core/gl_util.h>

inline constexpr size_t calc_size(const VertexAttribute attrs[]) {
    size_t vertexSize = 0;
    for (int i = 0; attrs[i].count; ++i) {
        vertexSize += attrs[i].size();
    }
    return vertexSize;
}

template<typename VertexStructure>
Mesh<VertexStructure>::Mesh(
    const MeshData<VertexStructure>& data
) : Mesh(
        data.vertices.data(),
        data.vertices.size(),
        data.indices.data(),
        data.indices.size()
    ) {}

template<typename VertexStructure>
Mesh<VertexStructure>::Mesh(
    const VertexStructure* vertexBuffer,
    size_t vertices,
    const uint32_t* indexBuffer,
    size_t indices
) : VAO(0),
    VBO(0),
    IBO(0),
    vertexCount(0),
    indexCount(0)
{
    static_assert(
        calc_size(VertexStructure::ATTRIBUTES) == sizeof(VertexStructure)
    );

    const auto& attrs = VertexStructure::ATTRIBUTES;
    MeshStats::meshesCount++;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    reload(vertexBuffer, vertices, indexBuffer, indices);

    int offset = 0;
    for (int i = 0; attrs[i].count; ++i) {
        const VertexAttribute& attr = attrs[i];
        glVertexAttribPointer(
            i,
            attr.count,
            gl::to_glenum(attr.type),
            attr.normalized,
            sizeof(VertexStructure),
            (GLvoid*)(size_t)offset
        );
        glEnableVertexAttribArray(i);
        offset += attr.size();
    }

    glBindVertexArray(0);
}

template<typename VertexStructure>
Mesh<VertexStructure>::~Mesh(){
    MeshStats::meshesCount--;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    if (IBO != 0) glDeleteBuffers(1, &IBO);
}

template<typename VertexStructure>
void Mesh<VertexStructure>::reload(
    const VertexStructure *vertexBuffer,
    size_t vertexCount,
    const uint32_t *indexBuffer,
    size_t indexCount
) {
    this->vertexCount = vertexCount;
    this->indexCount = indexCount;
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    if (vertexBuffer != nullptr && vertexCount != 0) {
        glBufferData(
            GL_ARRAY_BUFFER,
            vertexCount * sizeof(VertexStructure),
            vertexBuffer,
            GL_STREAM_DRAW
        );
    } else {
        glBufferData(GL_ARRAY_BUFFER, 0, {}, GL_STREAM_DRAW);
    }

    if (indexBuffer != nullptr && indexCount != 0) {
        if (IBO == 0) glGenBuffers(1, &IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            sizeof(uint32_t) * indexCount,
            indexBuffer,
            GL_STATIC_DRAW
        );
    } else if (IBO != 0) {
        glDeleteBuffers(1, &IBO);
    }

}

template<typename VertexStructure>
void Mesh<VertexStructure>::draw(unsigned int primitive) const {
    MeshStats::drawCalls++;
    glBindVertexArray(VAO);
    if (IBO != 0) {
        glDrawElements(primitive, indexCount, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(primitive, 0, vertexCount);
    }
    glBindVertexArray(0);
}

template<typename VertexStructure>
void Mesh<VertexStructure>::draw() const {
    draw(GL_TRIANGLES);
}
