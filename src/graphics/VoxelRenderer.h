#ifndef GRAPHICS_VOXELRENDERER_H_
#define GRAPHICS_VOXELRENDERER_H_

#include <stdlib.h>
#include <vector>

#include "Mesh.h"
#include "../typedefs.h"

class Mesh;
class Chunk;

// Константы для формата вершин
namespace VoxelRenderer_Conts {
    constexpr int CHUNK_VERTEX_SIZE = (3 + 2 + 4); // Формат: pos(3) + texcoord(2) + color(4)
    const vattr CHUNK_ATTRS[] = {{3}, {2}, {4}, {0}};
    constexpr float UVSIZE = 1.0f / 16.0f; // Размер одной текстуры в атласе 16x16
}

// Отвечает за преобразование воксельных данных в гравические меши
class VoxelRenderer {
public:
    std::vector<float> buffer;
    ubyte lights[27 * 4];

    VoxelRenderer();
    ~VoxelRenderer();

    const float* render(Chunk* chunk, const Chunk** chunks, size_t& size);
};

#endif // GRAPHICS_VOXELRENDERER_H_
