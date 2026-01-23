#include "Chunks.h"

#include <math.h>
#include <limits.h>

#include <glm/glm.hpp>

#include "Chunk.h"
#include "voxel.h"
#include "Block.h"
#include "WorldGenerator.h"
#include "../files/WorldFiles.h"
#include "../lighting/LightMap.h"
#include "../lighting/Lighting.h"
#include "../graphics/Mesh.h"
#include "../math/voxmaths.h"
#include "../world/LevelEvents.h"

// Конструктор
Chunks::Chunks(uint width, uint depth, int areaOffsetX, int areaOffsetZ, WorldFiles* worldFiles, LevelEvents* events) : width(width), depth(depth), areaOffsetX(areaOffsetX), areaOffsetZ(areaOffsetZ), events(events), worldFiles(worldFiles) {
    volume = (size_t)width * (size_t)depth;

    chunks = new std::shared_ptr<Chunk>[volume];
    chunksSecond = new std::shared_ptr<Chunk>[volume];

    for (size_t i = 0; i < volume; ++i) {
        chunks[i] = nullptr;
    }

    chunksCount = 0;
}

// Деструктор
Chunks::~Chunks() {
    for (int i = 0; i < volume; ++i) {
        chunks[i] = nullptr;
        chunksSecond[i] = nullptr;
    }
    delete[] chunks;
    delete[] chunksSecond;
}

bool Chunks::putChunk(std::shared_ptr<Chunk> chunk) {
	int x = chunk->chunk_x;
	int z = chunk->chunk_z;

	x -= areaOffsetX;
	z -= areaOffsetZ;

	if (x < 0 || z < 0 || x >= width || z >= depth) return false;

	chunks[z * width + x] = chunk;
    chunksCount++;
	return true;
}

// Получает чанк по координатам чанка
Chunk* Chunks::getChunk(int x, int z) {
    x -= areaOffsetX;
    z -= areaOffsetZ;

    if (x < 0 || z < 0 || x >= width || z >= depth) return nullptr;

    return chunks[z * width + x].get();
}

// Получает воксель по мировым коордианатам
voxel* Chunks::getVoxel(int x, int y, int z) {
    x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

	int cx = x / CHUNK_WIDTH;
	int cy = y / CHUNK_HEIGHT;
	int cz = z / CHUNK_DEPTH;

	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;

	if (cx < 0 || cy < 0 || cz < 0 || cx >= width || cy >= 1 || cz >= depth) return nullptr;

	std::shared_ptr<Chunk> chunk = chunks[cz * width + cx];
	if (chunk == nullptr) return nullptr;

	int lx = x - cx * CHUNK_WIDTH;
	int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;

	return &chunk->voxels[(ly * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx];
}

ubyte Chunks::getLight(int x, int y, int z, int channel){
	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

	int cx = floordiv(x, CHUNK_WIDTH);
    int cy = floordiv(y, CHUNK_HEIGHT);
    int cz = floordiv(z, CHUNK_DEPTH);

	if (cx < 0 || cy < 0 || cz < 0 || cx >= width || cy >= 1 || cz >= depth) return 0;

	std::shared_ptr<Chunk> chunk = chunks[cz * width + cx];
	if (chunk == nullptr) return 0;

	int lx = x - cx * CHUNK_WIDTH;
	int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;

	return chunk->light_map->get(lx, ly, lz, channel);
}

light_t Chunks::getLight(int x, int y, int z){
	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

	int cx = floordiv(x, CHUNK_WIDTH);
    int cy = floordiv(y, CHUNK_HEIGHT);
    int cz = floordiv(z, CHUNK_DEPTH);

	if (cx < 0 || cy < 0 || cz < 0 || cx >= width || cy >= 1 || cz >= depth) return 0;

	std::shared_ptr<Chunk> chunk = chunks[cz * width + cx];
	if (chunk == nullptr) return 0;

	int lx = x - cx * CHUNK_WIDTH;
    int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;

	return chunk->light_map->get(lx, ly, lz);
}

Chunk* Chunks::getChunkByVoxel(int x, int y, int z){
	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

	int cx = x / CHUNK_WIDTH;
	int cy = y / CHUNK_HEIGHT;
	int cz = z / CHUNK_DEPTH;

	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;

	if (cx < 0 || cy < 0 || cz < 0 || cx >= width || cy >= 1 || cz >= depth) return nullptr;

	return chunks[cz * width + cx].get();
}

// Устанавливает идентификатор вокселя по мировым координатам.
void Chunks::setVoxel(int x, int y, int z, blockid_t id, uint8_t states) {
    if (y < 0 || y >= CHUNK_HEIGHT) return;

    x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

	int cx = x / CHUNK_WIDTH;
	int cz = z / CHUNK_DEPTH;

	if (x < 0) cx--;
	if (z < 0) cz--;

	if (cx < 0 || cz < 0 || cx >= width || cz >= depth) return;

	Chunk* chunk = chunks[cz * width + cx].get();
	if (chunk == nullptr) return;

	int lx = x - cx * CHUNK_WIDTH;
	int lz = z - cz * CHUNK_DEPTH;

	chunk->voxels[(y * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx].id = id;
    chunk->voxels[(y * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx].states = states;
	chunk->setModified(true);
    chunk->setUnsaved(true);

	if (lx == 0 && (chunk = getChunk(cx+areaOffsetX-1, cz+areaOffsetZ))) chunk->setModified(true);
	if (lz == 0 && (chunk = getChunk(cx+areaOffsetX, cz+areaOffsetZ-1))) chunk->setModified(true);

	if (lx == CHUNK_WIDTH-1 && (chunk = getChunk(cx+areaOffsetX+1, cz+areaOffsetZ))) chunk->setModified(true);
	if (lz == CHUNK_DEPTH-1 && (chunk = getChunk(cx+areaOffsetX, cz+areaOffsetZ+1))) chunk->setModified(true);
}

bool Chunks::isObstacle(int x, int y, int z){
	voxel* vox = getVoxel(x, y, z);
	if (vox == nullptr) return true;
	return Block::blocks[vox->id].get()->obstacle;
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

	constexpr float inf = std::numeric_limits<float>::infinity();

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
		voxel* vox = getVoxel(ix, iy, iz); // Получение текущего вокселя

        // Проверка, является ли воксель непрозрачным
		if (vox == nullptr || Block::blocks[vox->id].get()->selectable){
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

			return vox;
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

void Chunks::translate(int dx, int dz){
    for (uint i = 0; i < volume; ++i){
		chunksSecond[i] = nullptr;
	}

    for (uint z = 0; z < depth; ++z){
        for (uint x = 0; x < width; ++x){
            std::shared_ptr<Chunk> chunk = chunks[z * depth + x];
            if (chunk == nullptr) continue;
            int nx = x - dx;
            int nz = z - dz;
            if (nx < 0 || nz < 0 || nx >= width || nz >= depth){
                events->trigger(CHUNK_HIDDEN, chunk.get());
                if (worldFiles) worldFiles->put(chunk.get());
                chunksCount--;
                continue;
            }
            chunksSecond[nz * width + nx] = chunk;
        }
    }
	
	std::shared_ptr<Chunk>* chunks_temp = chunks;
	chunks = chunksSecond;
	chunksSecond = chunks_temp;

	areaOffsetX += dx;
	areaOffsetZ += dz;
}

void Chunks::setCenter(int x, int z) {
	int chunk_x = x / CHUNK_WIDTH;
	int chunk_z = z / CHUNK_DEPTH;

	chunk_x -= areaOffsetX;
	chunk_z -= areaOffsetZ;

	if (x < 0) chunk_x--;
	if (z < 0) chunk_z--;

	chunk_x -= width / 2;
	chunk_z -= depth / 2;

	if (chunk_x || chunk_z) translate(chunk_x, chunk_z);
}

void Chunks::resize(uint newWidth, uint newDepth) {
    if (newWidth < width) {
        int delta = width - newWidth;
        translate(delta / 2, 0);
        translate(-delta, 0);
        translate(delta, 0);
    }

    if (newDepth < depth) {
        int delta = depth - newDepth;
        translate(0, delta / 2);
        translate(0, -delta);
        translate(0, delta);
    }

    const int newVolume = newWidth * newDepth;
	std::shared_ptr<Chunk>* newChunks = new std::shared_ptr<Chunk>[newVolume] {};
	std::shared_ptr<Chunk>* newChunksSecond = new std::shared_ptr<Chunk>[newVolume] {};
	for (int z = 0; z < depth && z < newDepth; z++) {
		for (int x = 0; x < width && x < newWidth; x++) {
			newChunks[z * newWidth + x] = chunks[z * width + x];
		}
	}
	delete[] chunks;
	delete[] chunksSecond;
	width = newWidth;
	depth = newDepth;
	volume = newVolume;
	chunks = newChunks;
	chunksSecond = newChunksSecond;
}

void Chunks::_setOffset(int x, int z){
	areaOffsetX = x;
	areaOffsetZ = z;
}

void Chunks::clear(){
	for (size_t i = 0; i < volume; ++i){
		Chunk* chunk = chunks[i].get();
        if (chunk) {
            events->trigger(CHUNK_HIDDEN, chunk);
        }
        chunks[i] = nullptr;
	}
    chunksCount = 0;
}
