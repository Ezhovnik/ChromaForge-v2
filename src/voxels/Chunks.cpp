#include "Chunks.h"

#include <math.h>
#include <limits.h>

#include <glm/glm.hpp>

#include "Chunk.h"
#include "voxel.h"

// Преобразует мировые координаты в чанковые
void worldToChunkCoords(int world_x, int world_y, int world_z, 
                        int& chunk_x, int& chunk_y, int& chunk_z) {
    chunk_x = world_x / CHUNK_WIDTH;
    chunk_y = world_y / CHUNK_HEIGHT;
    chunk_z = world_z / CHUNK_DEPTH;
    
    // Корректировка для отрицательных координат
    if (world_x < 0) chunk_x--;
    if (world_y < 0) chunk_y--;
    if (world_z < 0) chunk_z--;
}

// Конструктор
Chunks::Chunks(uint width, uint height, uint depth) : width(width), height(height), depth(depth) {
    volume = width * height * depth;
    chunks = new Chunk*[volume];

    int index = 0;
    for (int y = 0; y < height; ++y) {
        for (int z = 0; z < depth; ++z) {
            for (int x = 0; x < width; ++x) {
                Chunk* chunk = new Chunk(x, y, z);
                chunks[index] = chunk;
                index++;
            }
        }
    }
}

// Деструктор
Chunks::~Chunks() {
    for (int i = 0; i < volume; ++i) {
        delete chunks[i];
    }
    delete[] chunks;
}

// Получает чанк по координатам чанка
Chunk* Chunks::getChunk(int chunk_x, int chunk_y, int chunk_z) {
    if (chunk_x < 0 || chunk_y < 0 || chunk_z < 0 || chunk_x >= width || chunk_y >= height || chunk_z >= depth) {
        return nullptr;
    }

    return chunks[(chunk_y * depth + chunk_z) * width + chunk_x];
}

// Получает воксель по мировым коордианатам
voxel* Chunks::getVoxel(int x, int y, int z) {
    int chunk_x, chunk_y, chunk_z;
    worldToChunkCoords(x, y, z, chunk_x, chunk_y, chunk_z);

    Chunk* chunk = getChunk(chunk_x, chunk_y, chunk_z);
    if (chunk == nullptr) {
        return nullptr;
    }

    // Вычисление локальных координат вокселя внутри чанка
    int local_x = x - chunk_x * CHUNK_WIDTH;
    int local_y = y - chunk_y * CHUNK_HEIGHT;
    int local_z = z - chunk_z * CHUNK_DEPTH;

    return &chunk->voxels[(local_y * CHUNK_DEPTH + local_z) * CHUNK_WIDTH + local_x];
}

// Устанавливает идентификатор вокселя по мировым координатам.
void Chunks::setVoxel(int x, int y, int z, int id) {
    int chunk_x, chunk_y, chunk_z;
    worldToChunkCoords(x, y, z, chunk_x, chunk_y, chunk_z);

    Chunk* chunk = getChunk(chunk_x, chunk_y, chunk_z);
    if (chunk == nullptr) {
        return;
    }

    // Вычисление локальных координат
    int local_x = x - chunk_x * CHUNK_WIDTH;
    int local_y = y - chunk_y * CHUNK_HEIGHT;
    int local_z = z - chunk_z * CHUNK_DEPTH;

    // Установка ID вокселя
    chunk->voxels[(local_y * CHUNK_DEPTH + local_z) * CHUNK_WIDTH + local_x].id = id;
    chunk->needsUpdate = true; // Чанк нужно обновить

    // Обновление соседних чанков, если воксель находится на границе
    if (local_x == 0 && (chunk = getChunk(chunk_x - 1, chunk_y, chunk_z))) chunk->needsUpdate = true;
    if (local_y == 0 && (chunk = getChunk(chunk_x, chunk_y - 1, chunk_z))) chunk->needsUpdate = true;
    if (local_z == 0 && (chunk = getChunk(chunk_x, chunk_y, chunk_z - 1))) chunk->needsUpdate = true;

    if (local_x == CHUNK_WIDTH - 1 && (chunk = getChunk(chunk_x + 1, chunk_y, chunk_z))) chunk->needsUpdate = true;
    if (local_y == CHUNK_HEIGHT - 1 && (chunk = getChunk(chunk_x, chunk_y + 1, chunk_z))) chunk->needsUpdate = true;
    if (local_z == CHUNK_DEPTH - 1 && (chunk = getChunk(chunk_x, chunk_y, chunk_z + 1))) chunk->needsUpdate = true;
}

// Выполняет трассировку луча через воксельный мир.
// Использует алгоритм цифрового дифференциального анализа (DDA).
voxel* Chunks::rayCast(glm::vec3 start, glm::vec3 dir, float maxDist, glm::vec3& end, glm::vec3& norm, glm::vec3& iend) {
    float px = start.x;
	float py = start.y;
	float pz = start.z;

	float dx = dir.x;
	float dy = dir.y;
	float dz = dir.z;

	float t = 0.0f; // Текущее расстояние вдоль луча

    // Начальные целочисленные координаты вокселя
	int ix = floor(px);
	int iy = floor(py);
	int iz = floor(pz);

    // Определение шагов для каждой оси
	float stepx = (dx > 0.0f) ? 1.0f : -1.0f;
	float stepy = (dy > 0.0f) ? 1.0f : -1.0f;
	float stepz = (dz > 0.0f) ? 1.0f : -1.0f;

	float inf = std::numeric_limits<float>::infinity();

    // Вычисление приращений параметра t при движении на один воксель
	float txDelta = (dx == 0.0f) ? inf : abs(1.0f / dx);
	float tyDelta = (dy == 0.0f) ? inf : abs(1.0f / dy);
	float tzDelta = (dz == 0.0f) ? inf : abs(1.0f / dz);

    // Вычисление расстояний до ближайших границ вокселей
	float xdist = (stepx > 0) ? (ix + 1 - px) : (px - ix);
	float ydist = (stepy > 0) ? (iy + 1 - py) : (py - iy);
	float zdist = (stepz > 0) ? (iz + 1 - pz) : (pz - iz);

    // Вычисление параметра t для достижения границ
	float txMax = (txDelta < inf) ? txDelta * xdist : inf;
	float tyMax = (tyDelta < inf) ? tyDelta * ydist : inf;
	float tzMax = (tzDelta < inf) ? tzDelta * zdist : inf;

	int steppedIndex = -1; // Индекс оси, по которой произошел последний шаг

    // Основной цикл алгоритма DDA
	while (t <= maxDist){
		voxel* voxel = getVoxel(ix, iy, iz); // Получение текущего вокселя

        // Проверка, является ли воксель непрозрачным (id != 0)
		if (voxel == nullptr || voxel->id){
            // Найден непрозрачный воксель или достигнута граница мира
			end.x = px + t * dx;
			end.y = py + t * dy;
			end.z = pz + t * dz;

			iend.x = ix;
			iend.y = iy;
			iend.z = iz;

            // Вычисление нормали поверхности
			norm.x = norm.y = norm.z = 0.0f;
			if (steppedIndex == 0) norm.x = -stepx;
			if (steppedIndex == 1) norm.y = -stepy;
			if (steppedIndex == 2) norm.z = -stepz;

			return voxel;
		}

        // Определение следующей оси для шага
		if (txMax < tyMax) {
			if (txMax < tzMax) {
                // Шаг по оси X
				ix += stepx;
				t = txMax;
				txMax += txDelta;
				steppedIndex = 0;
			} else {
                // Шаг по оси Z
				iz += stepz;
				t = tzMax;
				tzMax += tzDelta;
				steppedIndex = 2;
			}
		} else {
			if (tyMax < tzMax) {
                // Шаг по оси Y
				iy += stepy;
				t = tyMax;
				tyMax += tyDelta;
				steppedIndex = 1;
			} else {
                // Шаг по оси Z
				iz += stepz;
				t = tzMax;
				tzMax += tzDelta;
				steppedIndex = 2;
			}
		}
	}

    // Луч не нашел непрозрачный воксель в пределах maxDist
	iend.x = ix;
	iend.y = iy;
	iend.z = iz;

	end.x = px + t * dx;
	end.y = py + t * dy;
	end.z = pz + t * dz;

	norm.x = norm.y = norm.z = 0.0f;
	return nullptr;
}
