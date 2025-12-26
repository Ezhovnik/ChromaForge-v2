#include "VoxelRenderer.h"

#include "Mesh.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"

// Константы для формата вершины
constexpr int VERTEX_SIZE = (3 + 2 + 1); 
constexpr int CHUNK_ATTRS[] = {3, 2, 1, 0}; // Атрибуты: позиция(3), UV(2), освещение(1)

// Округляет деление в меньшую сторону
inline int cdiv(int x, int a) {
    return (x < 0) ? (x / a - 1) : (x / a);
}

// Преобразует отрицательные локальные координаты в положительные
inline int local_neg(int x, int size) {
    return (x < 0) ? (size + x) : x;
}

inline int local_coord(int x, int size) {
    return (x >= size) ? (x - size) : local_neg(x, size);
}

// Получает чанка из массива ближайших соседей по координатам чанка
inline const Chunk* get_chunk(const Chunk* const* closes, int x, int y, int z) {
    int chunk_x = cdiv(x, CHUNK_WIDTH);
    int chunk_y = cdiv(y, CHUNK_HEIGHT);
    int chunk_z = cdiv(z, CHUNK_DEPTH);
    
    int index = ((chunk_y + 1) * 3 + (chunk_z + 1)) * 3 + (chunk_x + 1);
    return closes[index];
}

// Проверяет доступность чанка
inline bool is_chunk_available(const Chunk* const* closes, int x, int y, int z) {
    return get_chunk(closes, x, y, z) != nullptr;
}

// Получает воксель по мировым координатам
inline const voxel& get_voxel(const Chunk* const* closes, int x, int y, int z) {
    const Chunk* chunk = get_chunk(closes, x, y, z);
    int local_x = local_coord(x, CHUNK_WIDTH);
    int local_y = local_coord(y, CHUNK_HEIGHT);
    int local_z = local_coord(z, CHUNK_DEPTH);
    
    return chunk->voxels[(local_y * CHUNK_DEPTH + local_z) * CHUNK_WIDTH + local_x];
}

// Проверяет, занята ли позиция вокселем
inline bool is_blocked(const Chunk* const* closes, int x, int y, int z) {
    return (!is_chunk_available(closes, x, y, z)) || get_voxel(closes, x, y, z).id;
}

// Функция для добавления вершины в буфер
inline void add_vertex(float* buffer, size_t& index, float x, float y, float z, float u, float v, float l) {
    buffer[index++] = x;
    buffer[index++] = y;
    buffer[index++] = z;
    buffer[index++] = u;
    buffer[index++] = v;
    buffer[index++] = l;
}

// Конструктор
VoxelRenderer::VoxelRenderer(size_t capacity) : capacity(capacity) {
    buffer = new float[capacity * VERTEX_SIZE * 6];
}

// Деструктор
VoxelRenderer::~VoxelRenderer(){
    delete[] buffer;
}

// Генерирует меш для рендеринга чанка
Mesh* VoxelRenderer::render(Chunk* chunk, const Chunk** closes, bool useAmbientOcclusion){
    float aoFactor = 0.15f; // Коэффициент затемнения для Ambient Occlusion
    size_t index = 0; // Текущий индекс в буфере вершин

    for (int y = 0; y < CHUNK_HEIGHT; y++){
        for (int z = 0; z < CHUNK_DEPTH; z++){
            for (int x = 0; x < CHUNK_WIDTH; x++){
                // Получение текущего вокселя
                voxel vox = chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
                uint id = vox.id;

                // Пропускаем пустые воксели
                if (!id){
                    continue;
                }

                float light;

                // Расчет UV координат для текстурного атласа
                float uvsize = 1.0f / 16.0f;
                float u1 = (id % 16) * uvsize;
                float v1 = 1.0f - (1 + id / 16) * uvsize;
                float u2 = u1 + uvsize;
                float v2 = v1 + uvsize;

                // AO значения
                float a, b, c, d, e, f, g, h;
                a = b = c = d = e = f = g = h = 0.0f;

                // Верхняя грань (Y+)
                if (!is_blocked(closes, x, y + 1, z)){
                    light = 1.0f;

                    // Вычисляем AO значения для соседних вокселей
                    if (useAmbientOcclusion){
                        a = is_blocked(closes, x + 1, y + 1, z) * aoFactor;
                        b = is_blocked(closes, x, y + 1, z + 1) * aoFactor;
                        c = is_blocked(closes, x - 1, y + 1, z) * aoFactor;
                        d = is_blocked(closes, x, y + 1, z - 1) * aoFactor;

                        e = is_blocked(closes, x - 1, y + 1, z - 1) * aoFactor;
                        f = is_blocked(closes, x - 1, y + 1, z + 1) * aoFactor;
                        g = is_blocked(closes, x + 1, y + 1, z + 1) * aoFactor;
                        h = is_blocked(closes, x + 1, y + 1, z - 1) * aoFactor;
                    }

                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z - 0.5f, u2, v1, light * (1.0f - c - d - e));
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z + 0.5f, u2, v2, light * (1.0f - c - b - f));
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u1, v2, light * (1.0f - a - b - g));

                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z - 0.5f, u2, v1, light * (1.0f - c - d - e));
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u1, v2, light * (1.0f - a - b - g));
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z - 0.5f, u1, v1, light * (1.0f - a - d - h));
                }
                // Нижняя грань (Y-)
                if (!is_blocked(closes, x, y - 1, z)){
                    light = 0.75f;

                    // Вычисляем AO значения для соседних вокселей
                    if (useAmbientOcclusion){
                        a = is_blocked(closes, x + 1, y - 1, z) * aoFactor;
                        b = is_blocked(closes, x, y - 1, z + 1) * aoFactor;
                        c = is_blocked(closes, x - 1, y - 1, z) * aoFactor;
                        d = is_blocked(closes, x, y - 1, z - 1) * aoFactor;

                        e = is_blocked(closes, x - 1, y - 1, z - 1) * aoFactor;
                        f = is_blocked(closes, x - 1, y - 1, z + 1) * aoFactor;
                        g = is_blocked(closes, x + 1, y - 1, z + 1) * aoFactor;
                        h = is_blocked(closes, x + 1, y - 1, z - 1) * aoFactor;
                    }

                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u1, v1, light * (1.0f - c - d - e));
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z + 0.5f, u2, v2, light * (1.0f - a - b - g));
                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z + 0.5f, u1, v2, light * (1.0f - a - b - g));

                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u1, v1, light * (1.0f - c - d -e));
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z - 0.5f, u2, v1, light * (1.0f - a - d -h));
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z + 0.5f, u2, v2, light * (1.0f - a - b -g));
                }

                // Правая грань (X+)
                if (!is_blocked(closes, x + 1, y, z)){
                    light = 0.95f;

                    // Вычисляем AO значения для соседних вокселей
                    if (useAmbientOcclusion){
                        a = is_blocked(closes, x + 1, y + 1, z) * aoFactor;
                        b = is_blocked(closes, x + 1, y, z + 1) * aoFactor;
                        c = is_blocked(closes, x + 1, y - 1, z) * aoFactor;
                        d = is_blocked(closes, x + 1, y, z - 1) * aoFactor;

                        e = is_blocked(closes, x + 1, y - 1, z - 1) * aoFactor;
                        f = is_blocked(closes, x + 1, y - 1, z + 1) * aoFactor;
                        g = is_blocked(closes, x + 1, y + 1, z + 1) * aoFactor;
                        h = is_blocked(closes, x + 1, y + 1, z - 1) * aoFactor;
                    }

                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z - 0.5f, u2, v1, light * (1.0f - c - d - e));
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z - 0.5f, u2, v2, light * (1.0f - d - a - h));
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u1, v2, light * (1.0f - a - b - g));

                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z - 0.5f, u2, v1, light * (1.0f - c - d - e));
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u1, v2, light * (1.0f - a - b - g));
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z + 0.5f, u1, v1, light * (1.0f - b - c - f));
                }
                // Левая грань (X-)
                if (!is_blocked(closes, x - 1, y, z)){
                    light = 0.85f;

                    // Вычисляем AO значения для соседних вокселей
                    if (useAmbientOcclusion){
                        a = is_blocked(closes, x - 1, y + 1, z) * aoFactor;
                        b = is_blocked(closes, x - 1, y, z + 1) * aoFactor;
                        c = is_blocked(closes, x - 1, y - 1, z) * aoFactor;
                        d = is_blocked(closes, x - 1, y, z - 1) * aoFactor;

                        e = is_blocked(closes, x - 1, y - 1, z - 1) * aoFactor;
                        f = is_blocked(closes, x - 1, y - 1, z + 1) * aoFactor;
                        g = is_blocked(closes, x - 1, y + 1, z + 1) * aoFactor;
                        h = is_blocked(closes, x - 1, y + 1, z - 1) * aoFactor;
                    }

                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u1, v1, light * (1.0f - c - d - e));
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z + 0.5f, u2, v2, light * (1.0f - a - b - g));
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z - 0.5f, u1, v2, light * (1.0f - d - a - h));

                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u1, v1, light * (1.0f - c - d - e));
                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z + 0.5f, u2, v1, light * (1.0f - b - c - f));
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z + 0.5f, u2, v2, light * (1.0f - a - b - g));
                }

                // Передняя грань (Z+)
                if (!is_blocked(closes, x, y, z + 1)){
                    light = 0.9f;

                    // Вычисляем AO значения для соседних вокселей
                    if (useAmbientOcclusion){
                        a = is_blocked(closes, x, y + 1, z + 1) * aoFactor;
                        b = is_blocked(closes, x + 1, y, z + 1) * aoFactor;
                        c = is_blocked(closes, x, y - 1, z + 1) * aoFactor;
                        d = is_blocked(closes, x - 1, y, z + 1) * aoFactor;

                        e = is_blocked(closes, x - 1, y - 1, z + 1) * aoFactor;
                        f = is_blocked(closes, x + 1, y - 1, z + 1) * aoFactor;
                        g = is_blocked(closes, x + 1, y + 1, z + 1) * aoFactor;
                        h = is_blocked(closes, x - 1, y + 1, z + 1) * aoFactor;
                    }

                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z + 0.5f, u1, v1, light * (1.0f - c - d - e));
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u2, v2, light * (1.0f - a - b - g));
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z + 0.5f, u1, v2, light * (1.0f - a - d - h));

                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z + 0.5f, u1, v1, light * (1.0f - c - d - e));
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z + 0.5f, u2, v1, light * (1.0f - b - c - f));
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u2, v2, light * (1.0f - a - b - g));
                }
                // Задняя грань (Z-)
                if (!is_blocked(closes, x, y, z - 1)){
                    light = 0.8f;

                    // Вычисляем AO значения для соседних вокселей
                    if (useAmbientOcclusion){
                        a = is_blocked(closes, x, y + 1, z - 1) * aoFactor;
                        b = is_blocked(closes, x + 1, y, z - 1) * aoFactor;
                        c = is_blocked(closes, x, y - 1, z - 1) * aoFactor;
                        d = is_blocked(closes, x - 1, y, z - 1) * aoFactor;

                        e = is_blocked(closes, x - 1, y - 1, z - 1) * aoFactor;
                        f = is_blocked(closes, x + 1, y - 1, z - 1) * aoFactor;
                        g = is_blocked(closes, x + 1, y + 1, z - 1) * aoFactor;
                        h = is_blocked(closes, x - 1, y + 1, z - 1) * aoFactor;
                    }

                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u2, v1, light * (1.0f - c - d - e));
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z - 0.5f, u2, v2, light * (1.0f - a - d - h));
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z - 0.5f, u1, v2, light * (1.0f - a - b - g));

                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u2, v1, light * (1.0f - c - d - e));
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z - 0.5f, u1, v2, light * (1.0f - a - b - g));
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z - 0.5f, u1, v1, light * (1.0f - b - c - f));
                }
            }
        }
    }
    return new Mesh(buffer, index / VERTEX_SIZE, CHUNK_ATTRS);
}
