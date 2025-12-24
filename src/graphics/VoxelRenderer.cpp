#include "VoxelRenderer.h"

#include "Mesh.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"

const int VERTEX_SIZE = 3 + 2 + 1; // Размер одной вершины в количество float компонентов

// Проверяет, находится ли координата в пределах чанка
bool isIn(int x, int y, int z) {
    return x >= 0 && x < CHUNK_WIDTH && y >= 0 && y < CHUNK_HEIGHT && z >= 0 && z < CHUNK_DEPTH;
}

// Получает воксель по координатам внутри чанка
voxel getVoxel(Chunk* chunk, int x, int y, int z) {
    return chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
}

// Проверяет, занята ли данная позиция вокселем
bool isBlocked(Chunk* chunk, int x, int y, int z) {
    return isIn(x, y, z) && getVoxel(chunk, x, y, z).id;
}

// Добавляет вершину в буфер вершинных данных
void addVertex(float* buffer, size_t& idx, int x, int y, int z, float u, float v, float l) {
    // Позиция вершины
    buffer[idx + 0] = x;
    buffer[idx + 1] = y;
    buffer[idx + 2] = z;

    // Текстурные координаты
    buffer[idx + 3] = u;
    buffer[idx + 4] = v;

    // Уровень освещения
    buffer[idx + 5] = l;

    idx += VERTEX_SIZE; // Увеличение индекса на размер одной вершины
}

const int CHUNK_ATTRS[] = {3, 2, 1, 0}; // Формат атрибутов вершин для передачи в меш

// Конструктор
VoxelRenderer::VoxelRenderer(size_t capacity) : capacity(capacity) {
    buffer = new float[capacity * VERTEX_SIZE * 6]; // Выделение памяти для буфера вершинных данных
}

// Деструктор
VoxelRenderer::~VoxelRenderer() {
    delete[] buffer;
}

// Рендерит чанк, создавая графический меш из его вокселей.
Mesh* VoxelRenderer::render(Chunk* chunk) {
    size_t index = 0; // Текущий индекс в буфере вершин

    // Итерация по всем вокселям чанка в порядке Y-Z-X
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            for (int x = 0; x < CHUNK_WIDTH; ++x) {
                // Получение текущего вокселя
                voxel vox = getVoxel(chunk, x, y, z);
                uint id = vox.id;

                // Пропуск воздушных вокселей (id == 0)
                if (!id) {
                    continue;
                }

                float light; // Уровень освещения для грани

                // Вычисление текстурных координат на основе ID вокселя
                // Предполагается использование текстуры-атласа 16x16 блоков
                float uvsize = 1.0f / 16.0f; // Размер одного блока в текстурных координатах
                float u = (id % 16) * uvsize; // Горизонтальная позиция в атласе
                float v = 1.0f - (1 + id / 16) * uvsize; // Вертикальная позиция в атласе

                // Верхняя грань (+Y)
                if (!isBlocked(chunk, x, y + 1, z)){
					light = 1.0f;
					addVertex(buffer, index, x - 0.5f, y + 0.5f, z - 0.5f, u + uvsize, v, light);
					addVertex(buffer, index, x - 0.5f, y + 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
					addVertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u, v + uvsize, light);

					addVertex(buffer, index, x - 0.5f, y + 0.5f, z - 0.5f, u + uvsize, v, light);
					addVertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u, v + uvsize, light);
					addVertex(buffer, index, x + 0.5f, y + 0.5f, z - 0.5f, u, v, light);
				}
                // Нижняя грань (-Y)
				if (!isBlocked(chunk, x, y - 1, z)){
					light = 0.75f;
					addVertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u, v, light);
					addVertex(buffer, index, x + 0.5f, y - 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
					addVertex(buffer, index, x - 0.5f, y - 0.5f, z + 0.5f, u, v + uvsize, light);

					addVertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u, v, light);
					addVertex(buffer, index, x + 0.5f, y - 0.5f, z - 0.5f, u + uvsize, v, light);
					addVertex(buffer, index, x + 0.5f, y - 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
				}

                // Правая грань (+X)
				if (!isBlocked(chunk, x + 1, y, z)){
					light = 0.95f;
					addVertex(buffer, index, x + 0.5f, y - 0.5f, z - 0.5f, u + uvsize, v, light);
					addVertex(buffer, index, x + 0.5f, y + 0.5f, z - 0.5f, u + uvsize, v + uvsize, light);
					addVertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u, v + uvsize, light);

					addVertex(buffer, index, x + 0.5f, y - 0.5f, z - 0.5f, u + uvsize, v, light);
					addVertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u, v + uvsize, light);
					addVertex(buffer, index, x + 0.5f, y - 0.5f, z + 0.5f, u, v, light);
				}
                // Левая грань (-X)
				if (!isBlocked(chunk, x - 1, y, z)){
					light = 0.85f;
					addVertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u, v, light);
					addVertex(buffer, index, x - 0.5f, y + 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
					addVertex(buffer, index, x - 0.5f, y + 0.5f, z - 0.5f, u, v + uvsize, light);

					addVertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u, v, light);
					addVertex(buffer, index, x - 0.5f, y - 0.5f, z + 0.5f, u + uvsize, v, light);
					addVertex(buffer, index, x - 0.5f, y + 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
				}

                // Передняя грань (+Z)
				if (!isBlocked(chunk, x, y, z + 1)){
					light = 0.9f;
					addVertex(buffer, index, x - 0.5f, y - 0.5f, z + 0.5f, u, v, light);
					addVertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
					addVertex(buffer, index, x - 0.5f, y + 0.5f, z + 0.5f, u, v + uvsize, light);

					addVertex(buffer, index, x - 0.5f, y - 0.5f, z + 0.5f, u, v, light);
					addVertex(buffer, index, x + 0.5f, y - 0.5f, z + 0.5f, u + uvsize, v, light);
					addVertex(buffer, index, x + 0.5f, y + 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
				}
                // Задняя грань (-Z)
				if (!isBlocked(chunk, x, y, z - 1)){
					light = 0.8f;
					addVertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u + uvsize, v, light);
					addVertex(buffer, index, x - 0.5f, y + 0.5f, z - 0.5f, u + uvsize, v + uvsize, light);
					addVertex(buffer, index, x + 0.5f, y + 0.5f, z - 0.5f, u, v + uvsize, light);

					addVertex(buffer, index, x - 0.5f, y - 0.5f, z - 0.5f, u + uvsize, v, light);
					addVertex(buffer, index, x + 0.5f, y + 0.5f, z - 0.5f, u, v + uvsize, light);
					addVertex(buffer, index, x + 0.5f, y - 0.5f, z - 0.5f, u, v, light);
				}
            }
        }
    }

    return new Mesh(buffer, index / VERTEX_SIZE, CHUNK_ATTRS);
}
