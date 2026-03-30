#include "Chunks.h"

#include <math.h>
#include <limits.h>
#include <vector>

#include "Chunk.h"
#include "voxel.h"
#include "Block.h"
#include "WorldGenerator.h"
#include "../lighting/Lightmap.h"
#include "../files/WorldFiles.h"
#include "../graphics/core/Mesh.h"
#include "../math/voxmaths.h"
#include "../world/LevelEvents.h"
#include "../core_content_defs.h"
#include "../content/Content.h"
#include "../math/AABB.h"
#include "../math/rays.h"

// TODO: Refactor this garbage

Chunks::Chunks(
	uint32_t width, 
	uint32_t depth, 
	int32_t areaOffsetX, 
	int32_t areaOffsetZ, 
	WorldFiles* worldFiles, 
	LevelEvents* events, 
	const Content* content
) : width(width), 
	depth(depth), 
	areaOffsetX(areaOffsetX), 
	areaOffsetZ(areaOffsetZ), 
	events(events), 
	worldFiles(worldFiles), 
	contentIds(content->getIndices()), 
	chunks(width * depth), 
	chunksSecond(width * depth)
{
	volume = (size_t)width * (size_t)depth;
	chunksCount = 0;
}

voxel* Chunks::getVoxel(int32_t x, int32_t y, int32_t z){
	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

    int cx = floordiv(x, CHUNK_WIDTH);
    int cy = floordiv(y, CHUNK_HEIGHT);
    int cz = floordiv(z, CHUNK_DEPTH);
	
	if (cx < 0 || cy < 0 || cz < 0 || cx >= int(width) || cy >= 1 || cz >= int(depth)) return nullptr;

	std::shared_ptr<Chunk> chunk = chunks[(cy * depth + cz) * width + cx];
	if (chunk == nullptr) return nullptr;

	int lx = x - cx * CHUNK_WIDTH;
	int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;

	return &chunk->voxels[(ly * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx];
}

const AABB* Chunks::isObstacleAt(float x, float y, float z) {
	int ix = floor(x);
	int iy = floor(y);
	int iz = floor(z);
	voxel* vox = getVoxel(ix, iy, iz);

	if (vox == nullptr) {
        if (iy >= CHUNK_HEIGHT) {
			return nullptr;
		} else {
			static const AABB empty;
			return &empty;
		}
    }

	const Block* def = contentIds->getBlockDef(vox->id);
	if (def->obstacle) {
		const auto& boxes = def->rotatable ? def->rt.hitboxes[vox->rotation()] : def->hitboxes;
        for (const auto& hitbox : boxes) {
			if (hitbox.contains({x - ix, y - iy, z - iz})) return &hitbox;
		}
	}
	return nullptr;
}

bool Chunks::isSolidBlock(int32_t x, int32_t y, int32_t z) {
    voxel* vox = getVoxel(x, y, z);
    if (vox == nullptr) return false;
    return contentIds->getBlockDef(vox->id)->rt.solid;
}

bool Chunks::isReplaceableBlock(int32_t x, int32_t y, int32_t z) {
    voxel* v = getVoxel(x, y, z);
    if (v == nullptr) return false;
    return contentIds->getBlockDef(v->id)->replaceable;
}

bool Chunks::isObstacleBlock(int32_t x, int32_t y, int32_t z) {
	voxel* v = getVoxel(x, y, z);
	if (v == nullptr) return false;
	return contentIds->getBlockDef(v->id)->obstacle;
}

ubyte Chunks::getLight(int32_t x, int32_t y, int32_t z, int channel) {
	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

	int cx = floordiv(x, CHUNK_WIDTH);
	int cy = floordiv(y, CHUNK_HEIGHT);
	int cz = floordiv(z, CHUNK_DEPTH);

	if (cx < 0 || cy < 0 || cz < 0 || cx >= int(width) || cy >= 1 || cz >= int(depth)) return 0;

	std::shared_ptr<Chunk> chunk = chunks[(cy * depth + cz) * width + cx];
	if (chunk == nullptr) return 0;

	int lx = x - cx * CHUNK_WIDTH;
	int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;

	return chunk->light_map.get(lx, ly, lz, channel);
}

light_t Chunks::getLight(int32_t x, int32_t y, int32_t z){
	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

	int cx = floordiv(x, CHUNK_WIDTH);
	int cy = floordiv(y, CHUNK_HEIGHT);
	int cz = floordiv(z, CHUNK_DEPTH);

	if (cx < 0 || cy < 0 || cz < 0 || cx >= int(width) || cy >= 1 || cz >= int(depth)) return 0;

	std::shared_ptr<Chunk> chunk = chunks[(cy * depth + cz) * width + cx];
	if (chunk == nullptr) return 0;

	int lx = x - cx * CHUNK_WIDTH;
	int ly = y - cy * CHUNK_HEIGHT;
	int lz = z - cz * CHUNK_DEPTH;

	return chunk->light_map.get(lx, ly, lz);
}

Chunk* Chunks::getChunkByVoxel(int x, int y, int z) {
    if (y < 0 || y >= CHUNK_HEIGHT) return nullptr;

	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

    int cx = floordiv(x, CHUNK_WIDTH);
	int cz = floordiv(z, CHUNK_DEPTH);
	
	if (cx < 0 || cz < 0 || cx >= int(width) || cz >= int(depth)) return nullptr;
	return chunks[cz * width + cx].get();
}

Chunk* Chunks::getChunk(int32_t x, int32_t z) {
	x -= areaOffsetX;
	z -= areaOffsetZ;
	if (x < 0 || z < 0 || x >= int(width) || z >= int(depth)) return nullptr;
	return chunks[z * width + x].get();
}

void Chunks::setVoxel(int32_t x, int32_t y, int32_t z, blockid_t id, uint8_t states) {
    if (y < 0 || y >= CHUNK_HEIGHT) return;

	x -= areaOffsetX * CHUNK_WIDTH;
	z -= areaOffsetZ * CHUNK_DEPTH;

    int cx = floordiv(x, CHUNK_WIDTH);
    int cz = floordiv(z, CHUNK_DEPTH);

	if (cx < 0 || cz < 0 || cx >= int(width) || cz >= int(depth)) return;

	Chunk* chunk = chunks[cz * width + cx].get();
	if (chunk == nullptr) return;

	int lx = x - cx * CHUNK_WIDTH;
	int lz = z - cz * CHUNK_DEPTH;

	voxel& vox = chunk->voxels[(y * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx];
	auto def = contentIds->getBlockDef(vox.id);
	if (def->inventorySize == 0) chunk->removeBlockInventory(lx, y, lz);
	vox.id = id;
	vox.states = states;

	chunk->setUnsaved(true);
	chunk->setModified(true);

    if (y < chunk->bottom) chunk->bottom = y;
    else if (y + 1 > chunk->top) chunk->top = y + 1;
    else if (id == BLOCK_AIR) chunk->updateHeights();

	if (lx == 0 && (chunk = getChunk(cx + areaOffsetX - 1, cz + areaOffsetZ))) chunk->setModified(true);
	if (lz == 0 && (chunk = getChunk(cx + areaOffsetX, cz + areaOffsetZ - 1))) chunk->setModified(true);

	if (lx == CHUNK_WIDTH - 1 && (chunk = getChunk(cx + areaOffsetX + 1, cz + areaOffsetZ))) chunk->setModified(true);
	if (lz == CHUNK_DEPTH - 1 && (chunk = getChunk(cx + areaOffsetX, cz + areaOffsetZ + 1))) chunk->setModified(true);
}

voxel* Chunks::rayCast(glm::vec3 start, glm::vec3 dir, float maxDist, glm::vec3& end, glm::ivec3& norm, glm::ivec3& iend) {
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

	int stepx = (dx > 0.0f) ? 1 : -1;
	int stepy = (dy > 0.0f) ? 1 : -1;
	int stepz = (dz > 0.0f) ? 1 : -1;

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
		if (voxel == nullptr) return nullptr;
		const Block* def = contentIds->getBlockDef(voxel->id);
		if (def->selectable) {
			end.x = px + t * dx;
			end.y = py + t * dy;
			end.z = pz + t * dz;

			iend.x = ix;
			iend.y = iy;
			iend.z = iz;

			if (!def->rt.solid) {
				const std::vector<AABB>& hitboxes = def->rotatable ? def->rt.hitboxes[voxel->rotation()] : def->hitboxes;

                scalar_t distance = maxDist;
                Ray ray(start, dir);

				bool hit = false;

                for (const auto& box : hitboxes) {
                    scalar_t boxDistance;
					glm::ivec3 boxNorm;
                    if (ray.intersectAABB(iend, box, maxDist, boxNorm, boxDistance) > RayRelation::None && boxDistance < distance) {
                        hit = true;
                        distance = boxDistance;
                        norm = boxNorm;
                        end = start + (dir * glm::vec3(distance));
                    }
                }

				if (hit) return voxel;
			} else {
				iend.x = ix;
				iend.y = iy;
				iend.z = iz;

				norm.x = norm.y = norm.z = 0;
				if (steppedIndex == 0) norm.x = -stepx;
				if (steppedIndex == 1) norm.y = -stepy;
				if (steppedIndex == 2) norm.z = -stepz;
				return voxel;
			}
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
	norm.x = norm.y = norm.z = 0;
	return nullptr;
}

glm::vec3 Chunks::rayCastToObstacle(glm::vec3 start, glm::vec3 dir, float maxDist) {
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

	int stepx = (dx > 0.0f) ? 1 : -1;
	int stepy = (dy > 0.0f) ? 1 : -1;
	int stepz = (dz > 0.0f) ? 1 : -1;

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

	while (t <= maxDist) {
		voxel* voxel = getVoxel(ix, iy, iz);
		if (voxel == nullptr) return glm::vec3(px + t * dx, py + t * dy, pz + t * dz);

		const Block* def = contentIds->getBlockDef(voxel->id);
		if (def->obstacle) {
			if (!def->rt.solid) {
				const std::vector<AABB>& hitboxes = def->rotatable ? def->rt.hitboxes[voxel->rotation()] : def->modelBoxes;

                scalar_t distance;
                glm::ivec3 norm;
                Ray ray(start, dir);

                for (const auto& box : hitboxes) {
                    if (ray.intersectAABB(glm::ivec3(ix, iy, iz), box, maxDist, norm, distance) > RayRelation::None) {
                        return start + (dir * glm::vec3(distance));
                    }
                }
			} else {
				return glm::vec3(px + t * dx, py + t * dy, pz + t * dz);
			}
		}
		if (txMax < tyMax) {
			if (txMax < tzMax) {
				ix += stepx;
				t = txMax;
				txMax += txDelta;
			}
			else {
				iz += stepz;
				t = tzMax;
				tzMax += tzDelta;
			}
		} else {
			if (tyMax < tzMax) {
				iy += stepy;
				t = tyMax;
				tyMax += tyDelta;
			}
			else {
				iz += stepz;
				t = tzMax;
				tzMax += tzDelta;
			}
		}
	}
	return glm::vec3(px + maxDist * dx, py + maxDist * dy, pz + maxDist * dz);
}

void Chunks::setCenter(int32_t x, int32_t z) {
	int cx = floordiv(x, CHUNK_WIDTH);
	int cz = floordiv(z, CHUNK_DEPTH);

    cx -= areaOffsetX + width / 2;
    cz -= areaOffsetZ + depth / 2;

	if (cx | cz) translate(cx, cz);
}

void Chunks::translate(int32_t dx, int32_t dz) {
	auto& regions = worldFiles->getRegions();
	for (uint i = 0; i < volume; ++i){
		chunksSecond[i] = nullptr;
	}
	for (uint32_t z = 0; z < depth; ++z) {
		for (uint32_t x = 0; x < width; ++x) {
			std::shared_ptr<Chunk> chunk = chunks[z * width + x];
			int nx = x - dx;
			int nz = z - dz;
			if (chunk == nullptr) continue;
			if (nx < 0 || nz < 0 || nx >= int(width) || nz >= int(depth)) {
				events->trigger(CHUNK_HIDDEN, chunk.get());
				regions.put(chunk.get());
				chunksCount--;
				continue;
			}
			chunksSecond[nz * width + nx] = chunk;
		}
	}
	std::swap(chunks, chunksSecond);

	areaOffsetX += dx;
	areaOffsetZ += dz;
}

void Chunks::_setOffset(int32_t x, int32_t z){
	areaOffsetX = x;
	areaOffsetZ = z;
}

bool Chunks::putChunk(std::shared_ptr<Chunk> chunk) {
	int x = chunk->chunk_x;
	int z = chunk->chunk_z;
	x -= areaOffsetX;
	z -= areaOffsetZ;
	if (x < 0 || z < 0 || x >= int(width) || z >= int(depth)) return false;
	chunks[z * width + x] = chunk;
	chunksCount++;
	return true;
}

void Chunks::resize(uint32_t newWidth, uint32_t newDepth) {
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
	std::vector<std::shared_ptr<Chunk>> newChunks(newVolume);
	std::vector<std::shared_ptr<Chunk>> newChunksSecond(newVolume);
	for (int z = 0; z < int(depth) && z < int(newDepth); ++z) {
		for (int x = 0; x < int(width) && x < int(newWidth); ++x) {
			newChunks[z * newWidth + x] = chunks[z * width + x];
		}
	}
	width = newWidth;
	depth = newDepth;
	volume = newVolume;
	chunks = std::move(newChunks);
	chunksSecond = std::move(newChunksSecond);
}

void Chunks::saveAndClear() {
	auto& regions = worldFiles->getRegions();
	for (size_t i = 0; i < volume; ++i){
		Chunk* chunk = chunks[i].get();
		chunks[i] = nullptr;
		if (chunk == nullptr || !chunk->isLighted()) continue;

		bool lightsUnsaved = !chunk->isLoadedLights() && worldFiles->doesWriteLights();
        if (!chunk->isUnsaved() && !lightsUnsaved) continue;
        regions.put(chunk);
	}
	chunksCount = 0;
}
