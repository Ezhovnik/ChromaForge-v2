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
#include "../graphics/VoxelRenderer.h"
#include "../graphics/Mesh.h"

// Конструктор
Chunks::Chunks(uint width, uint height, uint depth, int areaOffsetX, int areaOffsetY, int areaOffsetZ) : width(width), height(height), depth(depth), areaOffsetX(areaOffsetX), areaOffsetY(areaOffsetY), areaOffsetZ(areaOffsetZ) {
    volume = width * height * depth;

    chunks = new Chunk*[volume];
    chunksSecond = new Chunk*[volume];

    meshes = new Mesh*[volume];
    meshesSecond = new Mesh*[volume];

    std::fill_n(chunks, volume, nullptr);
    std::fill_n(meshes, volume, nullptr);
}

// Деструктор
Chunks::~Chunks() {
    for (int i = 0; i < volume; ++i) {
        delete chunks[i];
        delete meshes[i];
    }
    delete[] chunks;
    delete[] meshes;
    delete[] chunksSecond;
    delete[] meshesSecond;
}

// Получает чанк по координатам чанка
Chunk* Chunks::getChunk(int x, int y, int z) {
    x -= areaOffsetX;
    y -= areaOffsetY;
    z -= areaOffsetZ;

    if (x < 0 || y < 0 || z < 0 || x >= width || y >= height || z >= depth) return nullptr;

    return chunks[(y * depth + z) * width + x];
}

// Получает воксель по мировым коордианатам
voxel* Chunks::getVoxel(int x, int y, int z) {
    x -= areaOffsetX * CHUNK_WIDTH;
	y -= areaOffsetY * CHUNK_HEIGHT;
	z -= areaOffsetZ * CHUNK_DEPTH;
	int cx = x / CHUNK_WIDTH;
	int cy = y / CHUNK_HEIGHT;
	int cz = z / CHUNK_DEPTH;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0 || cx >= width || cy >= height || cz >= depth) return nullptr;
	Chunk* chunk = chunks[(cy * depth + cz) * width + cx];
	if (chunk == nullptr) return nullptr;
	int lx = x - cx * CHUNK_WIDTH;
	int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;
	return &chunk->voxels[(ly * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx];
}

unsigned char Chunks::getLight(int x, int y, int z, int channel){
	x -= areaOffsetX * CHUNK_WIDTH;
	y -= areaOffsetY * CHUNK_HEIGHT;
	z -= areaOffsetZ * CHUNK_DEPTH;
	int cx = x / CHUNK_WIDTH;
	int cy = y / CHUNK_HEIGHT;
	int cz = z / CHUNK_DEPTH;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0 || cx >= width || cy >= height || cz >= depth) return 0;
	Chunk* chunk = chunks[(cy * depth + cz) * width + cx];
	if (chunk == nullptr) return 0;
	int lx = x - cx * CHUNK_WIDTH;
	int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;
	return chunk->light_map->get(lx,ly,lz, channel);
}

Chunk* Chunks::getChunkByVoxel(int x, int y, int z){
	x -= areaOffsetX * CHUNK_WIDTH;
	y -= areaOffsetY * CHUNK_HEIGHT;
	z -= areaOffsetZ * CHUNK_DEPTH;
	int cx = x / CHUNK_WIDTH;
	int cy = y / CHUNK_HEIGHT;
	int cz = z / CHUNK_DEPTH;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0 || cx >= width || cy >= height || cz >= depth) return nullptr;
	return chunks[(cy * depth + cz) * width + cx];
}

// Устанавливает идентификатор вокселя по мировым координатам.
void Chunks::setVoxel(int x, int y, int z, int id) {
    x -= areaOffsetX * CHUNK_WIDTH;
	y -= areaOffsetY * CHUNK_HEIGHT;
	z -= areaOffsetZ * CHUNK_DEPTH;
	int cx = x / CHUNK_WIDTH;
	int cy = y / CHUNK_HEIGHT;
	int cz = z / CHUNK_DEPTH;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0 || cx >= width || cy >= height || cz >= depth) return;
	Chunk* chunk = chunks[(cy * depth + cz) * width + cx];
	if (chunk == nullptr) return;
	int lx = x - cx * CHUNK_WIDTH;
	int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;
	chunk->voxels[(ly * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx].id = id;
	chunk->needsUpdate = true;

	if (lx == 0 && (chunk = getChunk(cx+areaOffsetX-1, cy+areaOffsetY, cz+areaOffsetZ))) chunk->needsUpdate = true;
	if (ly == 0 && (chunk = getChunk(cx+areaOffsetX, cy+areaOffsetY-1, cz+areaOffsetZ))) chunk->needsUpdate = true;
	if (lz == 0 && (chunk = getChunk(cx+areaOffsetX, cy+areaOffsetY, cz+areaOffsetZ-1))) chunk->needsUpdate = true;

	if (lx == CHUNK_WIDTH-1 && (chunk = getChunk(cx+areaOffsetX+1, cy+areaOffsetY, cz+areaOffsetZ))) chunk->needsUpdate = true;
	if (ly == CHUNK_HEIGHT-1 && (chunk = getChunk(cx+areaOffsetX, cy+areaOffsetY+1, cz+areaOffsetZ))) chunk->needsUpdate = true;
	if (lz == CHUNK_DEPTH-1 && (chunk = getChunk(cx+areaOffsetX, cy+areaOffsetY, cz+areaOffsetZ+1))) chunk->needsUpdate = true;
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

bool Chunks::isObstacle(int x, int y, int z){
	voxel* vox = getVoxel(x,y,z);
	if (vox == nullptr) return true;
	return Block::blocks[vox->id]->obstacle;
}

void Chunks::translate(WorldFiles* worldFiles, int dx, int dy, int dz){
    std::fill_n(chunksSecond, volume, nullptr);
    std::fill_n(meshesSecond, volume, nullptr);

	for (uint y = 0; y < height; ++y){
		for (uint z = 0; z < depth; ++z){
			for (uint x = 0; x < width; ++x){
				Chunk* chunk = chunks[(y * depth + z) * width + x];
				int nx = x - dx;
				int ny = y - dy;
				int nz = z - dz;
				if (chunk == nullptr) continue;
				Mesh* mesh = meshes[(y * depth + z) * width + x];
				if (nx < 0 || ny < 0 || nz < 0 || nx >= width || ny >= height || nz >= depth){
					worldFiles->put((const char*)chunk->voxels, chunk->chunk_x, chunk->chunk_z);
					delete chunk;
					delete mesh;
					continue;
				}
				meshesSecond[(ny * depth + nz) * width + nx] = mesh;
				chunksSecond[(ny * depth + nz) * width + nx] = chunk;
			}
		}
	}
	Chunk** chunks_temp = chunks;
	chunks = chunksSecond;
	chunksSecond = chunks_temp;

	Mesh** meshes_temp = meshes;
	meshes = meshesSecond;
	meshesSecond = meshes_temp;

	areaOffsetX += dx;
	areaOffsetY += dy;
	areaOffsetZ += dz;
}

void Chunks::setCenter(WorldFiles* worldFiles, int x, int y, int z) {
	int cx = x / CHUNK_WIDTH;
	int cy = y / CHUNK_HEIGHT;
	int cz = z / CHUNK_DEPTH;
	cx -= areaOffsetX;
	cy -= areaOffsetY;
	cz -= areaOffsetZ;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	cx -= width / 2;
	cy -= height / 2;
	cz -= depth / 2;
	if (cx != 0 || cy != 0 || cz != 0) translate(worldFiles, cx,cy,cz);
}

bool Chunks::loadVisible(WorldFiles* worldFiles){
	int nearX = 0;
	int nearY = 0;
	int nearZ = 0;
	int minDistance = INT_MAX;
	for (uint y = 0; y < height; ++y){
		for (uint z = 1; z < depth-1; ++z){
			for (uint x = 1; x < width-1; ++x){
				int index = (y * depth + z) * width + x;
				Chunk* chunk = chunks[index];
				if (chunk != nullptr) continue;
				int lx = x - width / 2;
				int ly = y - height / 2;
				int lz = z - depth / 2;
				int distance = (lx * lx + ly * ly + lz * lz);
				if (distance < minDistance){
					minDistance = distance;
					nearX = x;
					nearY = y;
					nearZ = z;
				}
			}
		}
	}

	int index = (nearY * depth + nearZ) * width + nearX;
	Chunk* chunk = chunks[index];
	if (chunk != nullptr) return false;
	chunk = new Chunk(nearX+areaOffsetX,nearY+areaOffsetY,nearZ+areaOffsetZ);
	if (!worldFiles->getChunk(chunk->chunk_x, chunk->chunk_z, (char*)chunk->voxels)){
		WorldGenerator::generate(chunk->voxels, chunk->chunk_x, chunk->chunk_y, chunk->chunk_z);
	}

	chunks[index] = chunk;
	Lighting::onChunkLoaded(areaOffsetX+nearX, areaOffsetY+nearY, areaOffsetZ+nearZ);
	return true;
}

bool Chunks::_buildMeshes(VoxelRenderer* renderer) {
	int nearX = 0;
	int nearY = 0;
	int nearZ = 0;
	int minDistance = INT_MAX;
	for (uint y = 0; y < height; y++){
		for (uint z = 1; z < depth - 1; z++){
			for (uint x = 1; x < width - 1; x++){
				int index = (y * depth + z) * width + x;
				Chunk* chunk = chunks[index];
				if (chunk == nullptr) continue;
				Mesh* mesh = meshes[index];
				if (mesh != nullptr && !chunk->needsUpdate) continue;
				int lx = x - width / 2;
				int ly = y - height / 2;
				int lz = z - depth / 2;
				int distance = (lx * lx + ly * ly + lz * lz);
				if (distance < minDistance){
					minDistance = distance;
					nearX = x;
					nearY = y;
					nearZ = z;
				}
			}
		}
	}

	int index = (nearY * depth + nearZ) * width + nearX;

	Chunk* closes[27];

	Chunk* chunk = chunks[index];
	if (chunk == nullptr) return false;
	Mesh* mesh = meshes[index];
	if (mesh == nullptr || chunk->needsUpdate){
		if (mesh != nullptr) delete mesh;
		if (chunk->isEmpty()){
			meshes[index] = nullptr;
			return false;
		}
		chunk->needsUpdate = false;
        std::fill_n(closes, 27, nullptr);
		for (size_t j = 0; j < volume; j++){
			Chunk* other = chunks[j];
			if (other == nullptr) continue;

			int delta_x = other->chunk_x - chunk->chunk_x;
			int delta_y = other->chunk_y - chunk->chunk_y;
			int delta_z = other->chunk_z - chunk->chunk_z;

			if (abs(delta_x) > 1 || abs(delta_y) > 1 || abs(delta_z) > 1) continue;

			delta_x += 1;
			delta_y += 1;
			delta_z += 1;
			closes[(delta_y * 3 + delta_z) * 3 + delta_x] = other;
		}
		mesh = renderer->render(chunk, (const Chunk**)closes);
		meshes[index] = mesh;
		return true;
	}
	return false;
}
