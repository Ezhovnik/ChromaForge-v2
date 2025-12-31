#include "VoxelRenderer.h"
#include "Mesh.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"
#include "../lighting/LightMap.h"

// Константы для формата вершин
constexpr int VERTEX_SIZE = (3 + 2 + 4); // Формат: pos(3) + texcoord(2) + color(4)
const int CHUNK_ATTRS[] = {3, 2, 4, 0};
constexpr float UVSIZE = 1.0f / 16.0f; // Размер одной текстуры в атласе 16x16

// Целочисленное деление с округлением вниз для отрицательных чисел
inline int cdiv(int x, int a) {
    return (x < 0) ? (x / a - 1) : (x / a);
}

// Преобразует глобальную отрицательную координату в локальную внутри чанка
inline int local_neg(int x, int size) {
    return (x < 0) ? (size + x) : x;
}

// Преобразует глобальную координату в локальную внутри чанка
inline int local(int x, int size) {
    return (x >= size) ? (x - size) : local_neg(x, size);
}

// Получает указатель на чанк в окружающих чанках для заданных мировых координат 
const Chunk* get_chunk(int x, int y, int z, const Chunk** closes) {
    // Определяем, в каком из окружающих 27 чанков находится воксель
    int chunk_x = cdiv(x, CHUNK_WIDTH);
    int chunk_y = cdiv(y, CHUNK_HEIGHT);
    int chunk_z = cdiv(z, CHUNK_DEPTH);
    
    // Индекс в массиве 3x3x3 (центр + окружающие)
    int index = ((chunk_y + 1) * 3 + (chunk_z + 1)) * 3 + (chunk_x + 1);
    return closes[index];
}

// Проверяет, существует ли чанк в окружающих для заданных координат
bool is_chunk(int x, int y, int z, const Chunk** closes) {
    return get_chunk(x, y, z, closes) != nullptr;
}

// Получает значение освещенности для вокселя
int get_light(int x, int y, int z, int channel, const Chunk** closes) {
    const Chunk* chunk = get_chunk(x, y, z, closes);
    if (!chunk) return 0;
    
    int local_x = local(x, CHUNK_WIDTH);
    int local_y = local(y, CHUNK_HEIGHT);
    int local_z = local(z, CHUNK_DEPTH);
    return chunk->light_map->get(local_x, local_y, local_z, channel);
}

// Получает данные вокселя по мировым координатам
voxel get_voxel(int x, int y, int z, const Chunk** closes) {
    const Chunk* chunk = get_chunk(x, y, z, closes);
    if (!chunk) {
        voxel empty;
        empty.id = 0;
        return empty;
    }
    
    int lx = local(x, CHUNK_WIDTH);
    int ly = local(y, CHUNK_HEIGHT);
    int lz = local(z, CHUNK_DEPTH);
    return chunk->voxels[(ly * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx];
}

// Проверяет, является ли соседний воксель блокирующим для отрисовки грани
bool is_blocked(int x, int y, int z, const Chunk** closes, unsigned char group) {
    const Chunk* chunk = get_chunk(x, y, z, closes);
    if (!chunk) return true;
    
    int local_x = local(x, CHUNK_WIDTH);
    int local_y = local(y, CHUNK_HEIGHT);
    int local_z = local(z, CHUNK_DEPTH);
    voxel vox = chunk->voxels[(local_y * CHUNK_DEPTH + local_z) * CHUNK_WIDTH + local_x];
    return Block::blocks[vox.id]->drawGroup == group;
}

// Настраивает текстурные координаты для грани блока
void setup_uv(int texture_id, float& u1, float& v1, float& u2, float& v2) {
    u1 = (texture_id % 16) * UVSIZE;
    v1 = 1.0f - ((1 + texture_id / 16) * UVSIZE);
    u2 = u1 + UVSIZE;
    v2 = v1 + UVSIZE;
}

// Добавляет вершину в буфер
void add_vertex(float* buffer, size_t& index, 
                float x, float y, float z,
                float u, float v,
                float r, float g, float b, float s) {
    buffer[index++] = x;
    buffer[index++] = y;
    buffer[index++] = z;

    buffer[index++] = u;
    buffer[index++] = v;

    buffer[index++] = r;
    buffer[index++] = g;
    buffer[index++] = b;
    buffer[index++] = s;
}

// Конструктор
VoxelRenderer::VoxelRenderer(size_t capacity) : capacity(capacity) {
    buffer = new float[capacity * VERTEX_SIZE * 6];
}

// Деструктор
VoxelRenderer::~VoxelRenderer() {
    delete[] buffer;
}

// Генерирует меш для чанка
Mesh* VoxelRenderer::render(Chunk* chunk, const Chunk** closes) {
    size_t index = 0;

    // Итерация по всем вокселям чанка
    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        for (int z = 0; z < CHUNK_DEPTH; z++) {
            for (int x = 0; x < CHUNK_WIDTH; x++) {
                voxel vox = chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
                uint32_t voxel_id = vox.id;

                // Пропускаем воздушные блоки (id = 0)
                if (!voxel_id) continue;

                float u1, v1, u2, v2;

                Block* block = Block::blocks[voxel_id];
                unsigned char group = block->drawGroup;

                // Левая грань (-X)
                if (!is_blocked(x - 1, y, z, closes, group)) {
                    setup_uv(block->textureFaces[0], u1, v1, u2, v2);

                    float lr = get_light(x - 1, y, z, 0, closes) / 15.0f;
                    float lg = get_light(x - 1, y, z, 1, closes) / 15.0f;
                    float lb = get_light(x - 1, y, z, 2, closes) / 15.0f;
                    float ls = get_light(x - 1, y, z, 3, closes) / 15.0f;
                    
                    float lr0 = (get_light(x - 1, y - 1, z - 1, 0, closes) + lr * 30.0f + 
                                get_light(x - 1, y, z - 1, 0, closes) + 
                                get_light(x - 1, y - 1, z, 0, closes)) / 75.0f;
                    float lr1 = (get_light(x - 1, y + 1, z + 1, 0, closes) + lr * 30.0f + 
                                get_light(x - 1, y, z + 1, 0, closes) + 
                                get_light(x - 1, y + 1, z, 0, closes)) / 75.0f;
                    float lr2 = (get_light(x - 1, y + 1, z - 1, 0, closes) + lr * 30.0f + 
                                get_light(x - 1, y, z - 1, 0, closes) + 
                                get_light(x - 1, y + 1, z, 0, closes)) / 75.0f;
                    float lr3 = (get_light(x - 1, y - 1, z + 1, 0, closes) + lr * 30.0f + 
                                get_light(x - 1, y, z + 1, 0, closes) + 
                                get_light(x - 1, y - 1, z, 0, closes)) / 75.0f;
                    
                    float lg0 = (get_light(x - 1, y - 1, z - 1, 1, closes) + lg * 30.0f + 
                                get_light(x - 1, y, z - 1, 1, closes) + 
                                get_light(x - 1, y - 1, z, 1, closes)) / 75.0f;
                    float lg1 = (get_light(x - 1, y + 1, z + 1, 1, closes) + lg * 30.0f + 
                                get_light(x - 1, y, z + 1, 1, closes) + 
                                get_light(x - 1, y + 1, z, 1, closes)) / 75.0f;
                    float lg2 = (get_light(x - 1, y + 1, z - 1, 1, closes) + lg * 30.0f + 
                                get_light(x - 1, y, z - 1, 1, closes) + 
                                get_light(x - 1, y + 1, z, 1, closes)) / 75.0f;
                    float lg3 = (get_light(x - 1, y - 1, z + 1, 1, closes) + lg * 30.0f + 
                                get_light(x - 1, y, z + 1, 1, closes) + 
                                get_light(x - 1, y - 1, z, 1, closes)) / 75.0f;
                    
                    float lb0 = (get_light(x - 1, y - 1, z - 1, 2, closes) + lb * 30.0f + 
                                get_light(x - 1, y, z - 1, 2, closes) + 
                                get_light(x - 1, y - 1, z, 2, closes)) / 75.0f;
                    float lb1 = (get_light(x - 1, y + 1, z + 1, 2, closes) + lb * 30.0f + 
                                get_light(x - 1, y, z + 1, 2, closes) + 
                                get_light(x - 1, y + 1, z, 2, closes)) / 75.0f;
                    float lb2 = (get_light(x - 1, y + 1, z - 1, 2, closes) + lb * 30.0f + 
                                get_light(x - 1, y, z - 1, 2, closes) + 
                                get_light(x - 1, y + 1, z, 2, closes)) / 75.0f;
                    float lb3 = (get_light(x - 1, y - 1, z + 1, 2, closes) + lb * 30.0f + 
                                get_light(x - 1, y, z + 1, 2, closes) + 
                                get_light(x - 1, y - 1, z, 2, closes)) / 75.0f;
                    
                    float ls0 = (get_light(x - 1, y - 1, z - 1, 3, closes) + ls * 30.0f + 
                                get_light(x - 1, y, z - 1, 3, closes) + 
                                get_light(x - 1, y - 1, z, 3, closes)) / 75.0f;
                    float ls1 = (get_light(x - 1, y + 1, z + 1, 3, closes) + ls * 30.0f + 
                                get_light(x - 1, y, z + 1, 3, closes) + 
                                get_light(x - 1, y + 1, z, 3, closes)) / 75.0f;
                    float ls2 = (get_light(x - 1, y + 1, z - 1, 3, closes) + ls * 30.0f + 
                                get_light(x - 1, y, z - 1, 3, closes) + 
                                get_light(x - 1, y + 1, z, 3, closes)) / 75.0f;
                    float ls3 = (get_light(x - 1, y - 1, z + 1, 3, closes) + ls * 30.0f + 
                                get_light(x - 1, y, z + 1, 3, closes) + 
                                get_light(x - 1, y - 1, z, 3, closes)) / 75.0f;

                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u1, v1, lr0, lg0, lb0, ls0);
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z + 0.5f, u2, v2, lr1, lg1, lb1, ls1);
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z - 0.5f, u1, v2, lr2, lg2, lb2, ls2);
                    
                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u1, v1, lr0, lg0, lb0, ls0);
                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z + 0.5f, u2, v1, lr3, lg3, lb3, ls3);
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z + 0.5f, u2, v2, lr1, lg1, lb1, ls1);
                }
                // Правая грань (+X)
                if (!is_blocked(x + 1, y, z, closes, group)) {
                    setup_uv(block->textureFaces[1], u1, v1, u2, v2);

                    float lr = get_light(x + 1, y, z, 0, closes) / 15.0f;
                    float lg = get_light(x + 1, y, z, 1, closes) / 15.0f;
                    float lb = get_light(x + 1, y, z, 2, closes) / 15.0f;
                    float ls = get_light(x + 1, y, z, 3, closes) / 15.0f;
                    
                    float lr0 = (get_light(x + 1, y - 1, z - 1, 0, closes) + lr * 30.0f + 
                                get_light(x + 1, y, z - 1, 0, closes) + 
                                get_light(x + 1, y - 1, z, 0, closes)) / 75.0f;
                    float lr1 = (get_light(x + 1, y + 1, z - 1, 0, closes) + lr * 30.0f + 
                                get_light(x + 1, y, z - 1, 0, closes) + 
                                get_light(x + 1, y + 1, z, 0, closes)) / 75.0f;
                    float lr2 = (get_light(x + 1, y + 1, z + 1, 0, closes) + lr * 30.0f + 
                                get_light(x + 1, y, z + 1, 0, closes) + 
                                get_light(x + 1, y + 1, z, 0, closes)) / 75.0f;
                    float lr3 = (get_light(x + 1, y - 1, z + 1, 0, closes) + lr * 30.0f + 
                                get_light(x + 1, y, z + 1, 0, closes) + 
                                get_light(x + 1, y - 1, z, 0, closes)) / 75.0f;
                    
                    float lg0 = (get_light(x + 1, y - 1, z - 1, 1, closes) + lg * 30.0f + 
                                get_light(x + 1, y, z - 1, 1, closes) + 
                                get_light(x + 1, y - 1, z, 1, closes)) / 75.0f;
                    float lg1 = (get_light(x + 1, y + 1, z - 1, 1, closes) + lg * 30.0f + 
                                get_light(x + 1, y, z - 1, 1, closes) + 
                                get_light(x + 1, y + 1, z, 1, closes)) / 75.0f;
                    float lg2 = (get_light(x + 1, y + 1, z + 1, 1, closes) + lg * 30.0f + 
                                get_light(x + 1, y, z + 1, 1, closes) + 
                                get_light(x + 1, y + 1, z, 1, closes)) / 75.0f;
                    float lg3 = (get_light(x + 1, y - 1, z + 1, 1, closes) + lg * 30.0f + 
                                get_light(x + 1, y, z + 1, 1, closes) + 
                                get_light(x + 1, y - 1, z, 1, closes)) / 75.0f;
                    
                    float lb0 = (get_light(x + 1, y - 1, z - 1, 2, closes) + lb * 30.0f + 
                                get_light(x + 1, y, z - 1, 2, closes) + 
                                get_light(x + 1, y - 1, z, 2, closes)) / 75.0f;
                    float lb1 = (get_light(x + 1, y + 1, z - 1, 2, closes) + lb * 30.0f + 
                                get_light(x + 1, y, z - 1, 2, closes) + 
                                get_light(x + 1, y + 1, z, 2, closes)) / 75.0f;
                    float lb2 = (get_light(x + 1, y + 1, z + 1, 2, closes) + lb * 30.0f + 
                                get_light(x + 1, y, z + 1, 2, closes) + 
                                get_light(x + 1, y + 1, z, 2, closes)) / 75.0f;
                    float lb3 = (get_light(x + 1, y - 1, z + 1, 2, closes) + lb * 30.0f + 
                                get_light(x + 1, y, z + 1, 2, closes) + 
                                get_light(x + 1, y - 1, z, 2, closes)) / 75.0f;
                    
                    float ls0 = (get_light(x + 1, y - 1, z - 1, 3, closes) + ls * 30.0f + 
                                get_light(x + 1, y, z - 1, 3, closes) + 
                                get_light(x + 1, y - 1, z, 3, closes)) / 75.0f;
                    float ls1 = (get_light(x + 1, y + 1, z - 1, 3, closes) + ls * 30.0f + 
                                get_light(x + 1, y, z - 1, 3, closes) + 
                                get_light(x + 1, y + 1, z, 3, closes)) / 75.0f;
                    float ls2 = (get_light(x + 1, y + 1, z + 1, 3, closes) + ls * 30.0f + 
                                get_light(x + 1, y, z + 1, 3, closes) + 
                                get_light(x + 1, y + 1, z, 3, closes)) / 75.0f;
                    float ls3 = (get_light(x + 1, y - 1, z + 1, 3, closes) + ls * 30.0f + 
                                get_light(x + 1, y, z + 1, 3, closes) + 
                                get_light(x + 1, y - 1, z, 3, closes)) / 75.0f;
                    
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z - 0.5f, u2, v1, lr0, lg0, lb0, ls0);
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z - 0.5f, u2, v2, lr1, lg1, lb1, ls1);
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u1, v2, lr2, lg2, lb2, ls2);
                    
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z - 0.5f, u2, v1, lr0, lg0, lb0, ls0);
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u1, v2, lr2, lg2, lb2, ls2);
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z + 0.5f, u1, v1, lr3, lg3, lb3, ls3);
                }
                
                // Нижняя грань (-Y)
                if (!is_blocked(x, y - 1, z, closes, group)) {
                    setup_uv(block->textureFaces[2], u1, v1, u2, v2);

                    float lr = get_light(x, y - 1, z, 0, closes) / 15.0f;
                    float lg = get_light(x, y - 1, z, 1, closes) / 15.0f;
                    float lb = get_light(x, y - 1, z, 2, closes) / 15.0f;
                    float ls = get_light(x, y - 1, z, 3, closes) / 15.0f;
                    
                    float lr0 = (get_light(x - 1, y - 1, z - 1, 0, closes) + lr * 30.0f + 
                                get_light(x - 1, y - 1, z, 0, closes) + 
                                get_light(x, y - 1, z - 1, 0, closes)) / 75.0f;
                    float lr1 = (get_light(x + 1, y - 1, z + 1, 0, closes) + lr * 30.0f + 
                                get_light(x + 1, y - 1, z, 0, closes) + 
                                get_light(x, y - 1, z + 1, 0, closes)) / 75.0f;
                    float lr2 = (get_light(x - 1, y - 1, z + 1, 0, closes) + lr * 30.0f + 
                                get_light(x - 1, y - 1, z, 0, closes) + 
                                get_light(x, y - 1, z + 1, 0, closes)) / 75.0f;
                    float lr3 = (get_light(x + 1, y - 1, z - 1, 0, closes) + lr * 30.0f + 
                                get_light(x + 1, y - 1, z, 0, closes) + 
                                get_light(x, y - 1, z - 1, 0, closes)) / 75.0f;
                    
                    float lg0 = (get_light(x - 1, y - 1, z - 1, 1, closes) + lg * 30.0f + 
                                get_light(x - 1, y - 1, z, 1, closes) + 
                                get_light(x, y - 1, z - 1, 1, closes)) / 75.0f;
                    float lg1 = (get_light(x + 1, y - 1, z + 1, 1, closes) + lg * 30.0f + 
                                get_light(x + 1, y - 1, z, 1, closes) + 
                                get_light(x, y - 1, z + 1, 1, closes)) / 75.0f;
                    float lg2 = (get_light(x - 1, y - 1, z + 1, 1, closes) + lg * 30.0f + 
                                get_light(x - 1, y - 1, z, 1, closes) + 
                                get_light(x, y - 1, z + 1, 1, closes)) / 75.0f;
                    float lg3 = (get_light(x + 1, y - 1, z - 1, 1, closes) + lg * 30.0f + 
                                get_light(x + 1, y - 1, z, 1, closes) + 
                                get_light(x, y - 1, z - 1, 1, closes)) / 75.0f;
                    
                    float lb0 = (get_light(x - 1, y - 1, z - 1, 2, closes) + lb * 30.0f + 
                                get_light(x - 1, y - 1, z, 2, closes) + 
                                get_light(x, y - 1, z - 1, 2, closes)) / 75.0f;
                    float lb1 = (get_light(x + 1, y - 1, z + 1, 2, closes) + lb * 30.0f + 
                                get_light(x + 1, y - 1, z, 2, closes) + 
                                get_light(x, y - 1, z + 1, 2, closes)) / 75.0f;
                    float lb2 = (get_light(x - 1, y - 1, z + 1, 2, closes) + lb * 30.0f + 
                                get_light(x - 1, y - 1, z, 2, closes) + 
                                get_light(x, y - 1, z + 1, 2, closes)) / 75.0f;
                    float lb3 = (get_light(x + 1, y - 1, z - 1, 2, closes) + lb * 30.0f + 
                                get_light(x + 1, y - 1, z, 2, closes) + 
                                get_light(x, y - 1, z - 1, 2, closes)) / 75.0f;
                    
                    float ls0 = (get_light(x - 1, y - 1, z - 1, 3, closes) + ls * 30.0f + 
                                get_light(x - 1, y - 1, z, 3, closes) + 
                                get_light(x, y - 1, z - 1, 3, closes)) / 75.0f;
                    float ls1 = (get_light(x + 1, y - 1, z + 1, 3, closes) + ls * 30.0f + 
                                get_light(x + 1, y - 1, z, 3, closes) + 
                                get_light(x, y - 1, z + 1, 3, closes)) / 75.0f;
                    float ls2 = (get_light(x - 1, y - 1, z + 1, 3, closes) + ls * 30.0f + 
                                get_light(x - 1, y - 1, z, 3, closes) + 
                                get_light(x, y - 1, z + 1, 3, closes)) / 75.0f;
                    float ls3 = (get_light(x + 1, y - 1, z - 1, 3, closes) + ls * 30.0f + 
                                get_light(x + 1, y - 1, z, 3, closes) + 
                                get_light(x, y - 1, z - 1, 3, closes)) / 75.0f;
                    
                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u1, v1, lr0, lg0, lb0, ls0);
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z + 0.5f, u2, v2, lr1, lg1, lb1, ls1);
                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z + 0.5f, u1, v2, lr2, lg2, lb2, ls2);
                    
                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u1, v1, lr0, lg0, lb0, ls0);
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z - 0.5f, u2, v1, lr3, lg3, lb3, ls3);
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z + 0.5f, u2, v2, lr1, lg1, lb1, ls1);
                }
                // Верхнаяя грань (+Y)
                if (!is_blocked(x, y + 1, z, closes, group)) {
                    setup_uv(block->textureFaces[3], u1, v1, u2, v2);

                    float lr = get_light(x, y + 1, z, 0, closes) / 15.0f;
                    float lg = get_light(x, y + 1, z, 1, closes) / 15.0f;
                    float lb = get_light(x, y + 1, z, 2, closes) / 15.0f;
                    float ls = get_light(x, y + 1, z, 3, closes) / 15.0f;
                    
                    float lr0 = (get_light(x - 1, y + 1, z, 0, closes) + lr * 30.0f + 
                                get_light(x - 1, y + 1, z - 1, 0, closes) + 
                                get_light(x, y + 1, z - 1, 0, closes)) / 75.0f;
                    float lr1 = (get_light(x - 1, y + 1, z, 0, closes) + lr * 30.0f + 
                                get_light(x - 1, y + 1, z + 1, 0, closes) + 
                                get_light(x, y + 1, z + 1, 0, closes)) / 75.0f;
                    float lr2 = (get_light(x + 1, y + 1, z, 0, closes) + lr * 30.0f + 
                                get_light(x + 1, y + 1, z + 1, 0, closes) + 
                                get_light(x, y + 1, z + 1, 0, closes)) / 75.0f;
                    float lr3 = (get_light(x + 1, y + 1, z, 0, closes) + lr * 30.0f + 
                                get_light(x + 1, y + 1, z - 1, 0, closes) + 
                                get_light(x, y + 1, z - 1, 0, closes)) / 75.0f;
                    
                    float lg0 = (get_light(x - 1, y + 1, z, 1, closes) + lg * 30.0f + 
                                get_light(x - 1, y + 1, z - 1, 1, closes) + 
                                get_light(x, y + 1, z - 1, 1, closes)) / 75.0f;
                    float lg1 = (get_light(x - 1, y + 1, z, 1, closes) + lg * 30.0f + 
                                get_light(x - 1, y + 1, z + 1, 1, closes) + 
                                get_light(x, y + 1, z + 1, 1, closes)) / 75.0f;
                    float lg2 = (get_light(x + 1, y + 1, z, 1, closes) + lg * 30.0f + 
                                get_light(x + 1, y + 1, z + 1, 1, closes) + 
                                get_light(x, y + 1, z + 1, 1, closes)) / 75.0f;
                    float lg3 = (get_light(x + 1, y + 1, z, 1, closes) + lg * 30.0f + 
                                get_light(x + 1, y + 1, z - 1, 1, closes) + 
                                get_light(x, y + 1, z - 1, 1, closes)) / 75.0f;
                    
                    float lb0 = (get_light(x - 1, y + 1, z, 2, closes) + lb * 30.0f + 
                                get_light(x - 1, y + 1, z - 1, 2, closes) + 
                                get_light(x, y + 1, z - 1, 2, closes)) / 75.0f;
                    float lb1 = (get_light(x - 1, y + 1, z, 2, closes) + lb * 30.0f + 
                                get_light(x - 1, y + 1, z + 1, 2, closes) + 
                                get_light(x, y + 1, z + 1, 2, closes)) / 75.0f;
                    float lb2 = (get_light(x + 1, y + 1, z, 2, closes) + lb * 30.0f + 
                                get_light(x + 1, y + 1, z + 1, 2, closes) + 
                                get_light(x, y + 1, z + 1, 2, closes)) / 75.0f;
                    float lb3 = (get_light(x + 1, y + 1, z, 2, closes) + lb * 30.0f + 
                                get_light(x + 1, y + 1, z - 1, 2, closes) + 
                                get_light(x, y + 1, z - 1, 2, closes)) / 75.0f;
                    
                    float ls0 = (get_light(x - 1, y + 1, z, 3, closes) + ls * 30.0f + 
                                get_light(x - 1, y + 1, z - 1, 3, closes) + 
                                get_light(x, y + 1, z - 1, 3, closes)) / 75.0f;
                    float ls1 = (get_light(x - 1, y + 1, z, 3, closes) + ls * 30.0f + 
                                get_light(x - 1, y + 1, z + 1, 3, closes) + 
                                get_light(x, y + 1, z + 1, 3, closes)) / 75.0f;
                    float ls2 = (get_light(x + 1, y + 1, z, 3, closes) + ls * 30.0f + 
                                get_light(x + 1, y + 1, z + 1, 3, closes) + 
                                get_light(x, y + 1, z + 1, 3, closes)) / 75.0f;
                    float ls3 = (get_light(x + 1, y + 1, z, 3, closes) + ls * 30.0f + 
                                get_light(x + 1, y + 1, z - 1, 3, closes) + 
                                get_light(x, y + 1, z - 1, 3, closes)) / 75.0f;
                    
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z - 0.5f, u2, v1, lr0, lg0, lb0, ls0);
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z + 0.5f, u2, v2, lr1, lg1, lb1, ls1);
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u1, v2, lr2, lg2, lb2, ls2);
                    
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z - 0.5f, u2, v1, lr0, lg0, lb0, ls0);
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u1, v2, lr2, lg2, lb2, ls2);
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z - 0.5f, u1, v1, lr3, lg3, lb3, ls3);
                }

                // Задняя грань (-Z)
                if (!is_blocked(x, y, z - 1, closes, group)) {
                    setup_uv(block->textureFaces[4], u1, v1, u2, v2);

                    float lr = get_light(x, y, z - 1, 0, closes) / 15.0f;
                    float lg = get_light(x, y, z - 1, 1, closes) / 15.0f;
                    float lb = get_light(x, y, z - 1, 2, closes) / 15.0f;
                    float ls = get_light(x, y, z - 1, 3, closes) / 15.0f;
                    
                    float lr0 = 0.8f * (get_light(x - 1, y - 1, z - 1, 0, closes) + lr * 30.0f + 
                                    get_light(x, y - 1, z - 1, 0, closes) + 
                                    get_light(x - 1, y, z - 1, 0, closes)) / 75.0f;
                    float lr1 = 0.8f * (get_light(x - 1, y + 1, z - 1, 0, closes) + lr * 30.0f + 
                                    get_light(x, y + 1, z - 1, 0, closes) + 
                                    get_light(x - 1, y, z - 1, 0, closes)) / 75.0f;
                    float lr2 = 0.8f * (get_light(x + 1, y + 1, z - 1, 0, closes) + lr * 30.0f + 
                                    get_light(x, y + 1, z - 1, 0, closes) + 
                                    get_light(x + 1, y, z - 1, 0, closes)) / 75.0f;
                    float lr3 = 0.8f * (get_light(x + 1, y - 1, z - 1, 0, closes) + lr * 30.0f + 
                                    get_light(x, y - 1, z - 1, 0, closes) + 
                                    get_light(x + 1, y, z - 1, 0, closes)) / 75.0f;
                    
                    float lg0 = 0.8f * (get_light(x - 1, y - 1, z - 1, 1, closes) + lg * 30.0f + 
                                    get_light(x, y - 1, z - 1, 1, closes) + 
                                    get_light(x - 1, y, z - 1, 1, closes)) / 75.0f;
                    float lg1 = 0.8f * (get_light(x - 1, y + 1, z - 1, 1, closes) + lg * 30.0f + 
                                    get_light(x, y + 1, z - 1, 1, closes) + 
                                    get_light(x - 1, y, z - 1, 1, closes)) / 75.0f;
                    float lg2 = 0.8f * (get_light(x + 1, y + 1, z - 1, 1, closes) + lg * 30.0f + 
                                    get_light(x, y + 1, z - 1, 1, closes) + 
                                    get_light(x + 1, y, z - 1, 1, closes)) / 75.0f;
                    float lg3 = 0.8f * (get_light(x + 1, y - 1, z - 1, 1, closes) + lg * 30.0f + 
                                    get_light(x, y - 1, z - 1, 1, closes) + 
                                    get_light(x + 1, y, z - 1, 1, closes)) / 75.0f;
                    
                    float lb0 = 0.8f * (get_light(x - 1, y - 1, z - 1, 2, closes) + lb * 30.0f + 
                                    get_light(x, y - 1, z - 1, 2, closes) + 
                                    get_light(x - 1, y, z - 1, 2, closes)) / 75.0f;
                    float lb1 = 0.8f * (get_light(x - 1, y + 1, z - 1, 2, closes) + lb * 30.0f + 
                                    get_light(x, y + 1, z - 1, 2, closes) + 
                                    get_light(x - 1, y, z - 1, 2, closes)) / 75.0f;
                    float lb2 = 0.8f * (get_light(x + 1, y + 1, z - 1, 2, closes) + lb * 30.0f + 
                                    get_light(x, y + 1, z - 1, 2, closes) + 
                                    get_light(x + 1, y, z - 1, 2, closes)) / 75.0f;
                    float lb3 = 0.8f * (get_light(x + 1, y - 1, z - 1, 2, closes) + lb * 30.0f + 
                                    get_light(x, y - 1, z - 1, 2, closes) + 
                                    get_light(x + 1, y, z - 1, 2, closes)) / 75.0f;
                    
                    float ls0 = 0.8f * (get_light(x - 1, y - 1, z - 1, 3, closes) + ls * 30.0f + 
                                    get_light(x, y - 1, z - 1, 3, closes) + 
                                    get_light(x - 1, y, z - 1, 3, closes)) / 75.0f;
                    float ls1 = 0.8f * (get_light(x - 1, y + 1, z - 1, 3, closes) + ls * 30.0f + 
                                    get_light(x, y + 1, z - 1, 3, closes) + 
                                    get_light(x - 1, y, z - 1, 3, closes)) / 75.0f;
                    float ls2 = 0.8f * (get_light(x + 1, y + 1, z - 1, 3, closes) + ls * 30.0f + 
                                    get_light(x, y + 1, z - 1, 3, closes) + 
                                    get_light(x + 1, y, z - 1, 3, closes)) / 75.0f;
                    float ls3 = 0.8f * (get_light(x + 1, y - 1, z - 1, 3, closes) + ls * 30.0f + 
                                    get_light(x, y - 1, z - 1, 3, closes) + 
                                    get_light(x + 1, y, z - 1, 3, closes)) / 75.0f;
                    
                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u2, v1, lr0, lg0, lb0, ls0);
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z - 0.5f, u2, v2, lr1, lg1, lb1, ls1);
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z - 0.5f, u1, v2, lr2, lg2, lb2, ls2);
                    
                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u2, v1, lr0, lg0, lb0, ls0);
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z - 0.5f, u1, v2, lr2, lg2, lb2, ls2);
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z - 0.5f, u1, v1, lr3, lg3, lb3, ls3);
                }
                // Передняя грань (+Z)
                if (!is_blocked(x, y, z + 1, closes, group)) {
                    setup_uv(block->textureFaces[5], u1, v1, u2, v2);

                    float lr = get_light(x, y, z + 1, 0, closes) / 15.0f;
                    float lg = get_light(x, y, z + 1, 1, closes) / 15.0f;
                    float lb = get_light(x, y, z + 1, 2, closes) / 15.0f;
                    float ls = get_light(x, y, z + 1, 3, closes) / 15.0f;
                    
                    float lr0 = 0.9f * (get_light(x - 1, y - 1, z + 1, 0, closes) + lr * 30.0f + 
                                    get_light(x, y - 1, z + 1, 0, closes) + 
                                    get_light(x - 1, y, z + 1, 0, closes)) / 75.0f;
                    float lr1 = 0.9f * (get_light(x + 1, y + 1, z + 1, 0, closes) + lr * 30.0f + 
                                    get_light(x, y + 1, z + 1, 0, closes) + 
                                    get_light(x + 1, y, z + 1, 0, closes)) / 75.0f;
                    float lr2 = 0.9f * (get_light(x - 1, y + 1, z + 1, 0, closes) + lr * 30.0f + 
                                    get_light(x, y + 1, z + 1, 0, closes) + 
                                    get_light(x - 1, y, z + 1, 0, closes)) / 75.0f;
                    float lr3 = 0.9f * (get_light(x + 1, y - 1, z + 1, 0, closes) + lr * 30.0f + 
                                    get_light(x, y - 1, z + 1, 0, closes) + 
                                    get_light(x + 1, y, z + 1, 0, closes)) / 75.0f;
                    
                    float lg0 = 0.9f * (get_light(x - 1, y - 1, z + 1, 1, closes) + lg * 30.0f + 
                                    get_light(x, y - 1, z + 1, 1, closes) + 
                                    get_light(x - 1, y, z + 1, 1, closes)) / 75.0f;
                    float lg1 = 0.9f * (get_light(x + 1, y + 1, z + 1, 1, closes) + lg * 30.0f + 
                                    get_light(x, y + 1, z + 1, 1, closes) + 
                                    get_light(x + 1, y, z + 1, 1, closes)) / 75.0f;
                    float lg2 = 0.9f * (get_light(x - 1, y + 1, z + 1, 1, closes) + lg * 30.0f + 
                                    get_light(x, y + 1, z + 1, 1, closes) + 
                                    get_light(x - 1, y, z + 1, 1, closes)) / 75.0f;
                    float lg3 = 0.9f * (get_light(x + 1, y - 1, z + 1, 1, closes) + lg * 30.0f + 
                                    get_light(x, y - 1, z + 1, 1, closes) + 
                                    get_light(x + 1, y, z + 1, 1, closes)) / 75.0f;
                    
                    float lb0 = 0.9f * (get_light(x - 1, y - 1, z + 1, 2, closes) + lb * 30.0f + 
                                    get_light(x, y - 1, z + 1, 2, closes) + 
                                    get_light(x - 1, y, z + 1, 2, closes)) / 75.0f;
                    float lb1 = 0.9f * (get_light(x + 1, y + 1, z + 1, 2, closes) + lb * 30.0f + 
                                    get_light(x, y + 1, z + 1, 2, closes) + 
                                    get_light(x + 1, y, z + 1, 2, closes)) / 75.0f;
                    float lb2 = 0.9f * (get_light(x - 1, y + 1, z + 1, 2, closes) + lb * 30.0f + 
                                    get_light(x, y + 1, z + 1, 2, closes) + 
                                    get_light(x - 1, y, z + 1, 2, closes)) / 75.0f;
                    float lb3 = 0.9f * (get_light(x + 1, y - 1, z + 1, 2, closes) + lb * 30.0f + 
                                    get_light(x, y - 1, z + 1, 2, closes) + 
                                    get_light(x + 1, y, z + 1, 2, closes)) / 75.0f;
                    
                    float ls0 = 0.9f * (get_light(x - 1, y - 1, z + 1, 3, closes) + ls * 30.0f + 
                                    get_light(x, y - 1, z + 1, 3, closes) + 
                                    get_light(x - 1, y, z + 1, 3, closes)) / 75.0f;
                    float ls1 = 0.9f * (get_light(x + 1, y + 1, z + 1, 3, closes) + ls * 30.0f + 
                                    get_light(x, y + 1, z + 1, 3, closes) + 
                                    get_light(x + 1, y, z + 1, 3, closes)) / 75.0f;
                    float ls2 = 0.9f * (get_light(x - 1, y + 1, z + 1, 3, closes) + ls * 30.0f + 
                                    get_light(x, y + 1, z + 1, 3, closes) + 
                                    get_light(x - 1, y, z + 1, 3, closes)) / 75.0f;
                    float ls3 = 0.9f * (get_light(x + 1, y - 1, z + 1, 3, closes) + ls * 30.0f + 
                                    get_light(x, y - 1, z + 1, 3, closes) + 
                                    get_light(x + 1, y, z + 1, 3, closes)) / 75.0f;
                    
                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z + 0.5f, u1, v1, lr0, lg0, lb0, ls0);
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u2, v2, lr1, lg1, lb1, ls1);
                    add_vertex(buffer, index, x - 0.5f, y + 0.5f, z + 0.5f, u1, v2, lr2, lg2, lb2, ls2);
                    

                    add_vertex(buffer, index, x - 0.5f, y - 0.5f, z + 0.5f, u1, v1, lr0, lg0, lb0, ls0);
                    add_vertex(buffer, index, x + 0.5f, y - 0.5f, z + 0.5f, u2, v1, lr3, lg3, lb3, ls3);
                    add_vertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u2, v2, lr1, lg1, lb1, ls1);
                }
            }
        }
    }
    
    return new Mesh(buffer, index / VERTEX_SIZE, CHUNK_ATTRS);
}
