#include "Chunks.h"

#include <math.h>
#include <limits.h>

#include "Chunk.h"
#include "voxel.h"
#include "Block.h"
#include "WorldGenerator.h"
#include "../lighting/Lightmap.h"
#include "../files/WorldFiles.h"
#include "../graphics/Mesh.h"
#include "../math/voxmaths.h"
#include "../world/LevelEvents.h"
#include "../definitions.h"
#include "../content/Content.h"

Chunks::Chunks(uint width, uint depth, int areaOffsetX, int areaOffsetZ, WorldFiles* worldFiles, LevelEvents* events, const Content* content) : width(width), depth(depth), areaOffsetX(areaOffsetX), areaOffsetZ(areaOffsetZ), events(events), worldFiles(worldFiles), content(content), contentIds(content->indices){
	volume = (size_t)width * (size_t)depth;
	chunks = new std::shared_ptr<Chunk>[volume];
	chunksSecond = new std::shared_ptr<Chunk>[volume];

	for (size_t i = 0; i < volume; i++){
		chunks[i] = nullptr;
	}
	chunksCount = 0;

	airID = content->require(DEFAULT_BLOCK_NAMESPACE"air")->id;
}

Chunks::~Chunks(){
	for (size_t i = 0; i < volume; i++){
		chunks[i] = nullptr;
	}
	delete[] chunks;
}

voxel* Chunks::getVoxel(int x, int y, int z){
	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

    int cx = floordiv(x, CHUNK_WIDTH);
    int cy = floordiv(y, CHUNK_HEIGHT);
    int cz = floordiv(z, CHUNK_DEPTH);
	
	if (cx < 0 || cy < 0 || cz < 0 || cx >= width || cy >= 1 || cz >= depth) return nullptr;

	std::shared_ptr<Chunk> chunk = chunks[(cy * depth + cz) * width + cx];
	if (chunk == nullptr) return nullptr;

	int lx = x - cx * CHUNK_WIDTH;
	int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;

	return &chunk->voxels[(ly * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx];
}

bool Chunks::isObstacle(int x, int y, int z){
	voxel* vox = getVoxel(x, y, z);
	if (vox == nullptr) return true;
	return contentIds->getBlockDef(vox->id)->obstacle;
}

ubyte Chunks::getLight(int x, int y, int z, int channel){
	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

	int cx = floordiv(x, CHUNK_WIDTH);
	int cy = floordiv(y, CHUNK_HEIGHT);
	int cz = floordiv(z, CHUNK_DEPTH);

	if (cx < 0 || cy < 0 || cz < 0 || cx >= width || cy >= 1 || cz >= depth) return 0;

	std::shared_ptr<Chunk> chunk = chunks[(cy * depth + cz) * width + cx];
	if (chunk == nullptr) return 0;

	int lx = x - cx * CHUNK_WIDTH;
	int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;

	return chunk->light_map->get(lx,ly,lz, channel);
}

light_t Chunks::getLight(int x, int y, int z){
	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

	int cx = floordiv(x, CHUNK_WIDTH);
	int cy = floordiv(y, CHUNK_HEIGHT);
	int cz = floordiv(z, CHUNK_DEPTH);

	if (cx < 0 || cy < 0 || cz < 0 || cx >= width || cy >= 1 || cz >= depth) return 0;

	std::shared_ptr<Chunk> chunk = chunks[(cy * depth + cz) * width + cx];
	if (chunk == nullptr) return 0;

	int lx = x - cx * CHUNK_WIDTH;
	int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;

	return chunk->light_map->get(lx,ly,lz);
}

Chunk* Chunks::getChunkByVoxel(int x, int y, int z){
    if (y < 0 || y >= CHUNK_HEIGHT) return nullptr;

	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

    int cx = floordiv(x, CHUNK_WIDTH);
	int cz = floordiv(z, CHUNK_DEPTH);
	
	if (cx < 0 || cz < 0 || cx >= width || cz >= depth) return nullptr;
	return chunks[cz * width + cx].get();
}

Chunk* Chunks::getChunk(int x, int z){
	x -= areaOffsetX;
	z -= areaOffsetZ;
	if (x < 0 || z < 0 || x >= width || z >= depth) return nullptr;
	return chunks[z * width + x].get();
}

void Chunks::setVoxel(int x, int y, int z, int id, uint8_t states){
    if (y < 0 || y >= CHUNK_HEIGHT) return;

	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

    int cx = floordiv(x, CHUNK_WIDTH);
    int cz = floordiv(z, CHUNK_DEPTH);
	
	if (cx < 0 || cz < 0 || cx >= width || cz >= depth) return;

	Chunk* chunk = chunks[cz * width + cx].get();
	if (chunk == nullptr) return;

	int lx = x - cx * CHUNK_WIDTH;
	int lz = z - cz * CHUNK_DEPTH;
	chunk->voxels[(y * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx].id = id;
	chunk->voxels[(y * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx].states = states;

	chunk->setUnsaved(true);
	chunk->setModified(true);

    if (y < chunk->bottom) chunk->bottom = y;
    else if (y + 1 > chunk->top) chunk->top = y + 1;
    else if (id == airID) chunk->updateHeights();

	if (lx == 0 && (chunk = getChunk(cx+areaOffsetX-1, cz+areaOffsetZ))) chunk->setModified(true);
	if (lz == 0 && (chunk = getChunk(cx+areaOffsetX, cz+areaOffsetZ-1))) chunk->setModified(true);

	if (lx == CHUNK_WIDTH-1 && (chunk = getChunk(cx+areaOffsetX+1, cz+areaOffsetZ))) chunk->setModified(true);
	if (lz == CHUNK_DEPTH-1 && (chunk = getChunk(cx+areaOffsetX, cz+areaOffsetZ+1))) chunk->setModified(true);
}

voxel* Chunks::rayCast(glm::vec3 start, glm::vec3 dir, float maxDist, glm::vec3& end, glm::vec3& norm, glm::vec3& iend) {
	float px = start.x;
	float py = start.y;
	float pz = start.z;

	float dx = dir.x;
	float dy = dir.y;
	float dz = dir.z;

	float t = 0.0f;
	int ix = floor(px);
	int iy = floor(py);
	int iz = floor(pz);

	float stepx = (dx > 0.0f) ? 1.0f : -1.0f;
	float stepy = (dy > 0.0f) ? 1.0f : -1.0f;
	float stepz = (dz > 0.0f) ? 1.0f : -1.0f;

	constexpr float infinity = std::numeric_limits<float>::infinity();

	float txDelta = (dx == 0.0f) ? infinity : abs(1.0f / dx);
	float tyDelta = (dy == 0.0f) ? infinity : abs(1.0f / dy);
	float tzDelta = (dz == 0.0f) ? infinity : abs(1.0f / dz);

	float xdist = (stepx > 0) ? (ix + 1 - px) : (px - ix);
	float ydist = (stepy > 0) ? (iy + 1 - py) : (py - iy);
	float zdist = (stepz > 0) ? (iz + 1 - pz) : (pz - iz);

	float txMax = (txDelta < infinity) ? txDelta * xdist : infinity;
	float tyMax = (tyDelta < infinity) ? tyDelta * ydist : infinity;
	float tzMax = (tzDelta < infinity) ? tzDelta * zdist : infinity;

	int steppedIndex = -1;

	while (t <= maxDist){
		voxel* voxel = getVoxel(ix, iy, iz);
		if (voxel == nullptr || contentIds->getBlockDef(voxel->id)->selectable){
			end.x = px + t * dx;
			end.y = py + t * dy;
			end.z = pz + t * dz;

			iend.x = ix;
			iend.y = iy;
			iend.z = iz;

			norm.x = norm.y = norm.z = 0.0f;
			if (steppedIndex == 0) norm.x = -stepx;
			if (steppedIndex == 1) norm.y = -stepy;
			if (steppedIndex == 2) norm.z = -stepz;
			return voxel;
		}
		if (txMax < tyMax) {
			if (txMax < tzMax) {
				ix += stepx;
				t = txMax;
				txMax += txDelta;
				steppedIndex = 0;
			} else {
				iz += stepz;
				t = tzMax;
				tzMax += tzDelta;
				steppedIndex = 2;
			}
		} else {
			if (tyMax < tzMax) {
				iy += stepy;
				t = tyMax;
				tyMax += tyDelta;
				steppedIndex = 1;
			} else {
				iz += stepz;
				t = tzMax;
				tzMax += tzDelta;
				steppedIndex = 2;
			}
		}
	}
	iend.x = ix;
	iend.y = iy;
	iend.z = iz;

	end.x = px + t * dx;
	end.y = py + t * dy;
	end.z = pz + t * dz;
	norm.x = norm.y = norm.z = 0.0f;
	return nullptr;
}

void Chunks::setCenter(int x, int z) {
	int cx = floordiv(x, CHUNK_WIDTH);
	int cz = floordiv(z, CHUNK_DEPTH);

    cx -= areaOffsetX + width / 2;
    cz -= areaOffsetZ + depth / 2;

	if (cx | cz) translate(cx, cz);
}

void Chunks::translate(int dx, int dz){
	for (uint i = 0; i < volume; i++){
		chunksSecond[i] = nullptr;
	}
	for (int z = 0; z < depth; z++){
		for (int x = 0; x < width; x++){
			std::shared_ptr<Chunk> chunk = chunks[z * depth + x];
			int nx = x - dx;
			int nz = z - dz;
			if (chunk == nullptr) continue;
			if (nx < 0 || nz < 0 || nx >= width || nz >= depth){
				events->trigger(CHUNK_HIDDEN, chunk.get());
				if (worldFiles) worldFiles->put(chunk.get());
				chunksCount--;
				continue;
			}
			chunksSecond[nz * width + nx] = chunk;
		}
	}
	std::shared_ptr<Chunk>* chunksTemp = chunks;
	chunks = chunksSecond;
	chunksSecond = chunksTemp;

	areaOffsetX += dx;
	areaOffsetZ += dz;
}

void Chunks::_setOffset(int x, int z){
	areaOffsetX = x;
	areaOffsetZ = z;
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
	const size_t newVolume = (size_t)newWidth * (size_t)newDepth;
	auto newChunks = new std::shared_ptr<Chunk>[newVolume] {};
	auto newChunksSecond = new std::shared_ptr<Chunk>[newVolume] {};
	for (int z = 0; z < depth && z < newDepth; ++z) {
		for (int x = 0; x < width && x < newWidth; ++x) {
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

void Chunks::saveAndClear(){
	for (size_t i = 0; i < volume; ++i){
		Chunk* chunk = chunks[i].get();
		if (chunk) {
			worldFiles->put(chunk);
			events->trigger(CHUNK_HIDDEN, chunk);
		}
		chunks[i] = nullptr;
	}
	chunksCount = 0;
}
