#ifndef GRAPHICS_VOXELRENDERER_H_
#define GRAPHICS_VOXELRENDERER_H_

#include <stdlib.h>

class Mesh;
class Chunk;

// Отвечает за преобразование воксельных данных в гравические меши
class VoxelRenderer {
    float* buffer; // Буфер для хранения вершинных данных перед созданием меша
    size_t capacity; // Максимальная вместимость буфера в количестве float-значений
public:
    VoxelRenderer(size_t capacity); // Конструктор
    ~VoxelRenderer(); // Деструктор

    Mesh* render(Chunk* chunk, const Chunk** closes, bool useAmbientOcclusion); // Создает графический меш из воксельного чанка
};

#endif // GRAPHICS_VOXELRENDERER_H_
