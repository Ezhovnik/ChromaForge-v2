#pragma once

#include <stdexcept>
#include <vector>

#include <typedefs.h>
#include <util/Buffer.h>

struct VertexAttribute {
    enum class Type {
        FLOAT,
        INT, UNSIGNED_INT,
        SHORT, UNSIGNED_SHORT,
        BYTE, UNSIGNED_BYTE
    };
    Type type = Type::FLOAT;

    bool normalized = false;
    ubyte count = 0;

    [[nodiscard]] constexpr uint32_t size() const {
        switch (type) {
            case Type::FLOAT:
                return count * sizeof(float);
            case Type::UNSIGNED_INT:
            case Type::INT:
                return count * sizeof(int32_t);
            case Type::UNSIGNED_SHORT:
            case Type::SHORT:
                return count * sizeof(int16_t);
            case Type::UNSIGNED_BYTE:
            case Type::BYTE:
                return count * sizeof(int8_t);
        }
        return 0;
    }
};

template<typename VertexStructure>
struct MeshData {
    util::Buffer<VertexStructure> vertices;
    std::vector<util::Buffer<uint32_t>> indices;
    util::Buffer<VertexAttribute> attrs;

    MeshData() = default;

    MeshData(
        util::Buffer<VertexStructure> vertices,
        std::vector<util::Buffer<uint32_t>> indices,
        util::Buffer<VertexAttribute> attrs
    ) : vertices(std::move(vertices)),
        indices(std::move(indices)),
        attrs(std::move(attrs)
    ) {}
};
