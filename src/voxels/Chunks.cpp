#include "Chunks.h"
#include "Chunk.h"
#include "voxel.h"
#include "Block.h"
#include "WorldGenerator.h"
#include "../lighting/Lightmap.h"
#include "../files/WorldFiles.h"
#include "../graphics/Mesh.h"
#include "../math/voxmaths.h"
#include "../world/LevelEvents.h"

#include <math.h>
#include <limits.h>

using glm::vec3;
using std::shared_ptr;

Chunks::Chunks(uint width, uint depth, int areaOffsetX, int areaOffsetZ, WorldFiles* worldFiles, LevelEvents* events) : width(width), depth(depth), areaOffsetX(areaOffsetX), areaOffsetZ(areaOffsetZ), events(events), worldFiles(worldFiles){
	volume = (size_t)width * (size_t)depth;
	chunks = new shared_ptr<Chunk>[volume];
	chunksSecond = new shared_ptr<Chunk>[volume];

	for (size_t i = 0; i < volume; i++){
		chunks[i] = nullptr;
	}
	chunksCount = 0;
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
	int cx = x / CHUNK_WIDTH;
	int cy = y / CHUNK_HEIGHT;
	int cz = z / CHUNK_DEPTH;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0 || cx >= width || cy >= 1 || cz >= depth) return nullptr;
	shared_ptr<Chunk> chunk = chunks[(cy * depth + cz) * width + cx];
	if (chunk == nullptr) return nullptr;
	int lx = x - cx * CHUNK_WIDTH;
	int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;
	return &chunk->voxels[(ly * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx];
}

bool Chunks::isObstacle(int x, int y, int z){
	voxel* v = getVoxel(x,y,z);
	if (v == nullptr) return true;
	return Block::blocks[v->id]->obstacle;
}

ubyte Chunks::getLight(int x, int y, int z, int channel){
	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;
	int cx = floordiv(x, CHUNK_WIDTH);
	int cy = floordiv(y, CHUNK_HEIGHT);
	int cz = floordiv(z, CHUNK_DEPTH);
	if (cx < 0 || cy < 0 || cz < 0 || cx >= width || cy >= 1 || cz >= depth) return 0;
	shared_ptr<Chunk> chunk = chunks[(cy * depth + cz) * width + cx];
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
	shared_ptr<Chunk> chunk = chunks[(cy * depth + cz) * width + cx];
	if (chunk == nullptr) return 0;
	int lx = x - cx * CHUNK_WIDTH;
	int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;
	return chunk->light_map->get(lx,ly,lz);
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
	return chunks[(cy * depth + cz) * width + cx].get();
}

Chunk* Chunks::getChunk(int x, int z){
	x -= areaOffsetX;
	z -= areaOffsetZ;
	if (x < 0 || z < 0 || x >= width || z >= depth) return nullptr;
	return chunks[z * width + x].get();
}

void Chunks::setVoxel(int x, int y, int z, int id, uint8_t states){
	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;
	int cx = x / CHUNK_WIDTH;
	if (y < 0 || y >= CHUNK_HEIGHT) return;
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
	chunk->setUnsaved(true);
	chunk->setModified(true);

	if (lx == 0 && (chunk = getChunk(cx+areaOffsetX-1, cz+areaOffsetZ))) chunk->setModified(true);
	if (lz == 0 && (chunk = getChunk(cx+areaOffsetX, cz+areaOffsetZ-1))) chunk->setModified(true);

	if (lx == CHUNK_WIDTH-1 && (chunk = getChunk(cx+areaOffsetX+1, cz+areaOffsetZ))) chunk->setModified(true);
	if (lz == CHUNK_DEPTH-1 && (chunk = getChunk(cx+areaOffsetX, cz+areaOffsetZ+1))) chunk->setModified(true);
}

voxel* Chunks::rayCast(vec3 a, vec3 dir, float maxDist, vec3& end, vec3& norm, vec3& iend) {
	float px = a.x;
	float py = a.y;
	float pz = a.z;

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
		if (voxel == nullptr || Block::blocks[voxel->id]->selectable){
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
	int cx = x / CHUNK_WIDTH;
	int cz = z / CHUNK_DEPTH;
	cx -= areaOffsetX;
	cz -= areaOffsetZ;
	if (x < 0) cx--;
	if (z < 0) cz--;
	cx -= width/2;
	cz -= depth/2;
	if (cx | cz) translate(cx, cz);
}

void Chunks::translate(int dx, int dz){
	for (uint i = 0; i < volume; i++){
		chunksSecond[i] = nullptr;
	}
	for (int z = 0; z < depth; z++){
		for (int x = 0; x < width; x++){
			shared_ptr<Chunk> chunk = chunks[z * depth + x];
			int nx = x - dx;
			int nz = z - dz;
			if (chunk == nullptr) continue;
			if (nx < 0 || nz < 0 || nx >= width || nz >= depth){
				events->trigger(CHUNK_HIDDEN, chunk.get());
				worldFiles->put(chunk.get());
				chunksCount--;
				continue;
			}
			chunksSecond[nz * width + nx] = chunk;
		}
	}
	shared_ptr<Chunk>* chunksTemp = chunks;
	chunks = chunksSecond;
	chunksSecond = chunksTemp;

	areaOffsetX += dx;
	areaOffsetZ += dz;
}

void Chunks::_setOffset(int x, int z){
	areaOffsetX = x;
	areaOffsetZ = z;
}

bool Chunks::putChunk(shared_ptr<Chunk> chunk) {
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
	shared_ptr<Chunk>* newChunks = new shared_ptr<Chunk>[newVolume] {};
	shared_ptr<Chunk>* newChunksSecond = new shared_ptr<Chunk>[newVolume] {};
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

void Chunks::clear(){
	for (size_t i = 0; i < volume; i++){
		Chunk* chunk = chunks[i].get();
		if (chunk) {
			events->trigger(CHUNK_HIDDEN, chunk);
		}
	}
	chunksCount = 0;
}
