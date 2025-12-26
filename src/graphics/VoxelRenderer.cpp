#include "VoxelRenderer.h"

#include "Mesh.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"

// Константы для формата вершины
const int VERTEX_SIZE = (3 + 2 + 1); 
const int CHUNK_ATTRS[] = {3, 2, 1, 0}; // Атрибуты: позиция(3), UV(2), освещение(1)

// Макросы для работы с координатами чанков и вокселей
#define CDIV(X, A) (((X) < 0) ? ((X) / (A) - 1) : ((X) / (A)))  // Целочисленное деление с корректной обработкой отрицательных чисел
#define LOCAL_NEG(X, SIZE) (((X) < 0) ? ((SIZE) + (X)) : (X)) // Корректировка отрицательных локальных координат
#define LOCAL(X, SIZE) ((X) >= (SIZE) ? ((X) - (SIZE)) : LOCAL_NEG(X, SIZE)) // Получение локальной координаты в пределах чанка

// Макросы для доступа к чанкам и проверки блоков
#define IS_CHUNK(X, Y, Z) (GET_CHUNK(X, Y, Z) != nullptr) // Проверка существования чанка
#define GET_CHUNK(X, Y, Z) (closes[((CDIV(Y, CHUNK_HEIGHT) + 1) * 3 + CDIV(Z, CHUNK_DEPTH) + 1) * 3 + CDIV(X, CHUNK_WIDTH) + 1]) // Получение чанка из массива соседей

#define VOXEL(X, Y, Z) (GET_CHUNK(X, Y, Z)->voxels[(LOCAL(Y, CHUNK_HEIGHT) * CHUNK_DEPTH + LOCAL(Z, CHUNK_DEPTH)) * CHUNK_WIDTH + LOCAL(X, CHUNK_WIDTH)]) // Получение вокселя по мировым координатам
#define IS_BLOCKED(X, Y, Z) ((!IS_CHUNK(X, Y, Z)) || VOXEL(X, Y, Z).id) // Проверка, заблокирована ли позиция (либо нет чанка, либо есть воксель)

#define VERTEX(INDEX, X, Y, Z, U, V, L) buffer[INDEX + 0] = (X);\
								  buffer[INDEX + 1] = (Y);\
								  buffer[INDEX + 2] = (Z);\
								  buffer[INDEX + 3] = (U);\
								  buffer[INDEX + 4] = (V);\
								  buffer[INDEX + 5] = (L);\
								  INDEX += VERTEX_SIZE;

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
    float aoFactor = 0.15f;
	size_t index = 0;

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

                // AO values
                float a, b, c, d, e, f, g, h;
				a = b = c = d = e = f = g = h = 0.0f;

                // Верхняя грань (Y+)
				if (!IS_BLOCKED(x, y + 1, z)){
					light = 1.0f;

                    if (useAmbientOcclusion){
						a = IS_BLOCKED(x + 1, y + 1, z) * aoFactor;
						b = IS_BLOCKED(x, y + 1, z + 1) * aoFactor;
						c = IS_BLOCKED(x - 1, y + 1, z) * aoFactor;
						d = IS_BLOCKED(x, y + 1, z - 1) * aoFactor;

						e = IS_BLOCKED(x - 1, y + 1, z - 1) * aoFactor;
						f = IS_BLOCKED(x - 1, y + 1, z + 1) * aoFactor;
						g = IS_BLOCKED(x + 1, y + 1, z + 1) * aoFactor;
						h = IS_BLOCKED(x + 1, y + 1, z - 1) * aoFactor;
					}

					VERTEX(index, x - 0.5f, y + 0.5f, z - 0.5f, u2, v1, light * (1.0f-c-d-e));
					VERTEX(index, x - 0.5f, y + 0.5f, z + 0.5f, u2, v2, light * (1.0f-c-b-f));
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u1, v2, light * (1.0f-a-b-g));

					VERTEX(index, x - 0.5f, y + 0.5f, z - 0.5f, u2, v1, light * (1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u1, v2, light * (1.0f-a-b-g));
					VERTEX(index, x + 0.5f, y + 0.5f, z - 0.5f, u1, v1, light * (1.0f-a-d-h));
				}
                // Нижняя грань (Y-)
				if (!IS_BLOCKED(x, y - 1, z)){
					light = 0.75f;

                    if (useAmbientOcclusion){
						a = IS_BLOCKED(x+1,y-1,z)*aoFactor;
						b = IS_BLOCKED(x,y-1,z+1)*aoFactor;
						c = IS_BLOCKED(x-1,y-1,z)*aoFactor;
						d = IS_BLOCKED(x,y-1,z-1)*aoFactor;

						e = IS_BLOCKED(x-1,y-1,z-1)*aoFactor;
						f = IS_BLOCKED(x-1,y-1,z+1)*aoFactor;
						g = IS_BLOCKED(x+1,y-1,z+1)*aoFactor;
						h = IS_BLOCKED(x+1,y-1,z-1)*aoFactor;
					}

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u1, v1, light * (1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y - 0.5f, z + 0.5f, u2, v2, light * (1.0f-a-b-g));
					VERTEX(index, x - 0.5f, y - 0.5f, z + 0.5f, u1, v2, light * (1.0f-a-b-g));

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u1, v1, light * (1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y - 0.5f, z - 0.5f, u2, v1, light * (1.0f-a-d-h));
					VERTEX(index, x + 0.5f, y - 0.5f, z + 0.5f, u2, v2, light * (1.0f-a-b-g));
				}

                // Правая грань (X+)
				if (!IS_BLOCKED(x + 1, y, z)){
					light = 0.95f;

                    if (useAmbientOcclusion){
						a = IS_BLOCKED(x+1,y+1,z)*aoFactor;
						b = IS_BLOCKED(x+1,y,z+1)*aoFactor;
						c = IS_BLOCKED(x+1,y-1,z)*aoFactor;
						d = IS_BLOCKED(x+1,y,z-1)*aoFactor;

						e = IS_BLOCKED(x+1,y-1,z-1)*aoFactor;
						f = IS_BLOCKED(x+1,y-1,z+1)*aoFactor;
						g = IS_BLOCKED(x+1,y+1,z+1)*aoFactor;
						h = IS_BLOCKED(x+1,y+1,z-1)*aoFactor;
					}

					VERTEX(index, x + 0.5f, y - 0.5f, z - 0.5f, u2, v1, light * (1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y + 0.5f, z - 0.5f, u2, v2, light * (1.0f-d-a-h));
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u1, v2, light * (1.0f-a-b-g));

					VERTEX(index, x + 0.5f, y - 0.5f, z - 0.5f, u2, v1, light * (1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u1, v2, light * (1.0f-a-b-g));
					VERTEX(index, x + 0.5f, y - 0.5f, z + 0.5f, u1, v1, light * (1.0f-b-c-f));
				}
                // Левая грань (X-)
				if (!IS_BLOCKED(x - 1, y, z)){
					light = 0.85f;

                    if (useAmbientOcclusion){
						a = IS_BLOCKED(x-1,y+1,z)*aoFactor;
						b = IS_BLOCKED(x-1,y,z+1)*aoFactor;
						c = IS_BLOCKED(x-1,y-1,z)*aoFactor;
						d = IS_BLOCKED(x-1,y,z-1)*aoFactor;

						e = IS_BLOCKED(x-1,y-1,z-1)*aoFactor;
						f = IS_BLOCKED(x-1,y-1,z+1)*aoFactor;
						g = IS_BLOCKED(x-1,y+1,z+1)*aoFactor;
						h = IS_BLOCKED(x-1,y+1,z-1)*aoFactor;
					}

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u1, v1, light * (1.0f-c-d-e));
					VERTEX(index, x - 0.5f, y + 0.5f, z + 0.5f, u2, v2, light * (1.0f-a-b-g));
					VERTEX(index, x - 0.5f, y + 0.5f, z - 0.5f, u1, v2, light * (1.0f-d-a-h));

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u1, v1, light * (1.0f-c-d-e));
					VERTEX(index, x - 0.5f, y - 0.5f, z + 0.5f, u2, v1, light * (1.0f-b-c-f));
					VERTEX(index, x - 0.5f, y + 0.5f, z + 0.5f, u2, v2, light * (1.0f-a-b-g));
				}

                // Передняя грань (Z+)
				if (!IS_BLOCKED(x, y, z + 1)){
					light = 0.9f;

                    if (useAmbientOcclusion){
						a = IS_BLOCKED(x,y+1,z+1)*aoFactor;
						b = IS_BLOCKED(x+1,y,z+1)*aoFactor;
						c = IS_BLOCKED(x,y-1,z+1)*aoFactor;
						d = IS_BLOCKED(x-1,y,z+1)*aoFactor;

						e = IS_BLOCKED(x-1,y-1,z+1)*aoFactor;
						f = IS_BLOCKED(x+1,y-1,z+1)*aoFactor;
						g = IS_BLOCKED(x+1,y+1,z+1)*aoFactor;
						h = IS_BLOCKED(x-1,y+1,z+1)*aoFactor;
					}

					VERTEX(index, x - 0.5f, y - 0.5f, z + 0.5f, u1, v1, light *(1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u2, v2, light *(1.0f-a-b-g));
					VERTEX(index, x - 0.5f, y + 0.5f, z + 0.5f, u1, v2, light *(1.0f-a-d-h));

					VERTEX(index, x - 0.5f, y - 0.5f, z + 0.5f, u1, v1, light *(1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y - 0.5f, z + 0.5f, u2, v1, light *(1.0f-b-c-f));
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u2, v2, light *(1.0f-a-b-g));
				}
                // Задняя грань (Z-)
				if (!IS_BLOCKED(x, y, z - 1)){
					light = 0.8f;

                    if (useAmbientOcclusion){
						a = IS_BLOCKED(x,y+1,z-1)*aoFactor;
						b = IS_BLOCKED(x+1,y,z-1)*aoFactor;
						c = IS_BLOCKED(x,y-1,z-1)*aoFactor;
						d = IS_BLOCKED(x-1,y,z-1)*aoFactor;

						e = IS_BLOCKED(x-1,y-1,z-1)*aoFactor;
						f = IS_BLOCKED(x+1,y-1,z-1)*aoFactor;
						g = IS_BLOCKED(x+1,y+1,z-1)*aoFactor;
						h = IS_BLOCKED(x-1,y+1,z-1)*aoFactor;
					}

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u2, v1, light * (1.0f-c-d-e));
					VERTEX(index, x - 0.5f, y + 0.5f, z - 0.5f, u2, v2, light * (1.0f-a-d-h));
					VERTEX(index, x + 0.5f, y + 0.5f, z - 0.5f, u1, v2, light * (1.0f-a-b-g));

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u2, v1, light * (1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y + 0.5f, z - 0.5f, u1, v2, light * (1.0f-a-b-g));
					VERTEX(index, x + 0.5f, y - 0.5f, z - 0.5f, u1, v1, light * (1.0f-b-c-f));
				}
			}
		}
	}
	return new Mesh(buffer, index / VERTEX_SIZE, CHUNK_ATTRS);
}
