#pragma once

#include <GL/glew.h>

#include <graphics/core/ImageData.h>
#include <graphics/core/commons.h>
#include <graphics/core/MeshData.h>

namespace gl {
    inline GLenum to_glenum(ImageFormat imageFormat) {
        switch (imageFormat) {
            case ImageFormat::rgb888: return GL_RGB;
            case ImageFormat::rgba8888: return GL_RGBA;
            default: 
                return 0;
        }
    }

    inline GLenum to_glenum(DrawPrimitive primitive) {
        static const GLenum primitives[]{
            GL_POINTS,
            GL_LINES,
            GL_TRIANGLES
        };
        return primitives[static_cast<int>(primitive)];
    }

    inline GLenum to_glenum(VertexAttribute::Type type) {
        switch (type) {
            case VertexAttribute::Type::FLOAT:
                return GL_FLOAT;
            case VertexAttribute::Type::UNSIGNED_INT:
                return GL_UNSIGNED_INT;
            case VertexAttribute::Type::INT:
                return GL_INT;
            case VertexAttribute::Type::UNSIGNED_SHORT:
                return GL_UNSIGNED_SHORT;
            case VertexAttribute::Type::SHORT:
                return GL_SHORT;
            case VertexAttribute::Type::UNSIGNED_BYTE:
                return GL_UNSIGNED_BYTE;
            case VertexAttribute::Type::BYTE:
                return GL_BYTE;
        }
        return 0;
    }
}
