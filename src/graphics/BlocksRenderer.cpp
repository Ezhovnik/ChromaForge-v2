#include "BlocksRenderer.h"

#include <glm/glm.hpp>

#include "Mesh.h"
#include "UVRegion.h"
#include "../constants.h"
#include "../voxels/Block.h"
#include "../voxels/Chunk.h"
#include "../voxels/VoxelsVolume.h"
#include "../voxels/ChunksStorage.h"
#include "../lighting/Lightmap.h"
#include "../definitions.h"

namespace BlocksRenderer_Consts {
    inline constexpr int VERTEX_SIZE = 9;
    const vattr ATTRS[] = {{3}, {2}, {4}, {0}};
}

BlocksRenderer::BlocksRenderer(size_t capacity) : offset(0), capacity(capacity) {
	buffer = new float[capacity];
	voxelsBuffer = new VoxelsVolume(CHUNK_WIDTH + 2, CHUNK_HEIGHT, CHUNK_DEPTH + 2);
}


BlocksRenderer::~BlocksRenderer() {
	delete voxelsBuffer;
	delete[] buffer;
}

void BlocksRenderer::vertex(glm::vec3 coord, float u, float v, glm::vec4 light) {
	buffer[offset++] = coord.x;
	buffer[offset++] = coord.y;
	buffer[offset++] = coord.z;

	buffer[offset++] = u;
	buffer[offset++] = v;

	buffer[offset++] = light.r;
	buffer[offset++] = light.g;
	buffer[offset++] = light.b;
	buffer[offset++] = light.a;
}

void BlocksRenderer::face(glm::vec3 coord, float w, float h,
	const glm::vec3 axisX,
	const glm::vec3 axisY,
	const UVRegion& region,
	const glm::vec4 lights[4],
	const glm::vec4 tint) {
	if (offset + BlocksRenderer_Consts::VERTEX_SIZE * 6 > capacity) {
		overflow = true;
		return;
	}
	vertex(coord, region.u1, region.v1, lights[0] * tint);
	vertex(coord + axisX * w, region.u2, region.v1, lights[1] * tint);
	vertex(coord + axisX * w + axisY * h, region.u2, region.v2, lights[2] * tint);

	vertex(coord, region.u1, region.v1, lights[0] * tint);
	vertex(coord + axisX * w + axisY * h, region.u2, region.v2, lights[2] * tint);
	vertex(coord + axisY * h, region.u1, region.v2, lights[3] * tint);
}

void BlocksRenderer::face(glm::vec3 coord, float w, float h,
	const glm::vec3 axisX,
	const glm::vec3 axisY,
	const UVRegion& region,
	const glm::vec4 lights[4],
	const glm::vec4 tint,
	bool rotated) {
	if (offset + BlocksRenderer_Consts::VERTEX_SIZE * 6 > capacity) {
		overflow = true;
		return;
	}
	if (rotated) {
		vertex(coord, region.u2, region.v1, lights[0] * tint);
		vertex(coord + axisX * w, region.u2, region.v2, lights[1] * tint);
		vertex(coord + axisX * w + axisY * h, region.u1, region.v2, lights[2] * tint);

		vertex(coord, region.u2, region.v1, lights[0] * tint);
		vertex(coord + axisX * w + axisY * h, region.u1, region.v2, lights[2] * tint);
		vertex(coord + axisY * h, region.u1, region.v1, lights[3] * tint);
	}
	else {
		vertex(coord, region.u1, region.v1, lights[0] * tint);
		vertex(coord + axisX * w, region.u2, region.v1, lights[1] * tint);
		vertex(coord + axisX * w + axisY * h, region.u2, region.v2, lights[2] * tint);

		vertex(coord, region.u1, region.v1, lights[0] * tint);
		vertex(coord + axisX * w + axisY * h, region.u2, region.v2, lights[2] * tint);
		vertex(coord + axisY * h, region.u1, region.v2, lights[3] * tint);
	}
}

void BlocksRenderer::cube(glm::vec3 coord, glm::vec3 size, const UVRegion texfaces[6]) {
	glm::vec4 lights[]{glm::vec4(), glm::vec4(), glm::vec4(), glm::vec4()};

	face(coord, size.x, size.y, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), texfaces[0], lights);
	face(coord + glm::vec3(size.x, 0, -size.z), size.x, size.y, glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), texfaces[1], lights);

	face(coord + glm::vec3(0, size.y, 0), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), texfaces[2], lights);
	face(coord + glm::vec3(0, 0, -size.z), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), texfaces[3], lights);

	face(coord + glm::vec3(0, 0, -size.z), size.z, size.y, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), texfaces[4], lights);
	face(coord + glm::vec3(size.x, 0, 0), size.z, size.y, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), texfaces[5], lights);
}

constexpr glm::vec4 do_tint(float value) {
	return glm::vec4(value, value, value, 1.0f);
}

void BlocksRenderer::blockCube(int x, int y, int z, glm::vec3 size, const UVRegion texfaces[6], ubyte group) {
	glm::vec4 lights[]{glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f)};
	if (isOpen(x, y, z + 1, group)) {
		face(glm::vec3(x, y, z), size.x, size.y, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), texfaces[5], lights, do_tint(0.9f));
	}
	if (isOpen(x, y, z - 1, group)) {
		face(glm::vec3(x + size.x, y, z - size.z), size.x, size.y, glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), texfaces[4], lights, glm::vec4(1.0f));
	}

	if (isOpen(x, y + 1, z, group)) {
		face(glm::vec3(x, y + size.y, z), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), texfaces[3], lights);
	}

	if (isOpen(x, y - 1, z, group)) {
		face(glm::vec3(x, y, z - size.z), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), texfaces[2], lights, glm::vec4(1.0f));
	}

	if (isOpen(x - 1, y, z, group)) {
		face(glm::vec3(x, y, z - size.z), size.z, size.y, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), texfaces[0], lights, glm::vec4(1.0f));
	}
	if (isOpen(x + 1, y, z, group)) {
		face(glm::vec3(x + size.x, y, z), size.z, size.y, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), texfaces[1], lights, glm::vec4(1.0f));
	}
}

void BlocksRenderer::blockXSprite(int x, int y, int z, glm::vec3 size, const UVRegion texface1, const UVRegion texface2, float spread) {
	glm::vec4 lights[]{
			pickSoftLight(x, y, z, {1, 0, 0}, {0, 1, 0}),
			pickSoftLight(x + 1, y, z, {1, 0, 0}, {0, 1, 0}),
			pickSoftLight(x + 1, y + 1, z, {1, 0, 0}, {0, 1, 0}),
			pickSoftLight(x, y + 1, z, {1, 0, 0}, {0, 1, 0}) };

	int rand = ((x * z + y) ^ (z * y - x)) * (z + y);

	float xs = ((float)(char)rand / 512) * spread;
	float zs = ((float)(char)(rand >> 8) / 512) * spread;

	const float w = size.x/1.41f;
	face(glm::vec3(x + xs + (1.0 - w) * 0.5f, y, 
		      z + zs - 1 + (1.0 - w) * 0.5f), w, size.y, 
		      glm::vec3(1.0f, 0, 1.0f), glm::vec3(0, 1, 0), texface1, lights, do_tint(0.9f));
	face(glm::vec3(x + xs - (1.0 - w) * 0.5f + 1, y, 
		      z + zs - (1.0 - w) * 0.5f), w, size.y, 
		      glm::vec3(-1.0f, 0, -1.0f), glm::vec3(0, 1, 0), texface1, lights, do_tint(0.9f));

	face(glm::vec3(x + xs + (1.0 - w) * 0.5f, y, 
		      z + zs - (1.0 - w) * 0.5f), w, size.y, 
		      glm::vec3(1.0f, 0, -1.0f), glm::vec3(0, 1, 0), texface2, lights, do_tint(0.9f));
	face(glm::vec3(x + xs - (1.0 - w) * 0.5f + 1, y, 
		      z + zs + (1.0 - w) * 0.5f - 1), w, size.y, 
			  glm::vec3(-1.0f, 0, 1.0f), glm::vec3(0, 1, 0), texface2, lights, do_tint(0.9f));
}

void BlocksRenderer::blockCubeShaded(int x, int y, int z, glm::vec3 size, const UVRegion texfaces_[6], const Block* block, ubyte states) {
	ubyte group = block->drawGroup;
	UVRegion texfaces[6];
	int rot = 0;

	for (int i = 0; i < 6; ++i) {
		texfaces[i] = texfaces_[i];
	}

	if (block->rotatable) {
		if (states == 0x31) {
			rot = 1;
			texfaces[0] = texfaces_[2];
			texfaces[1] = texfaces_[3];
			texfaces[2] = texfaces_[0];
			texfaces[3] = texfaces_[1];
		} else if (states == 0x32) {
			rot = 2;
		} else if (states == 0x33) {
			rot = 3;
			texfaces[2] = texfaces_[4];
			texfaces[3] = texfaces_[5];
			texfaces[4] = texfaces_[2];
			texfaces[5] = texfaces_[3];
		}
	}
	if (isOpen(x, y, z + 1, group)) {
		glm::vec4 lights[]{
				pickSoftLight(x, y, z + 1, {1, 0, 0}, {0, 1, 0}),
				pickSoftLight(x + 1, y, z + 1, {1, 0, 0}, {0, 1, 0}),
				pickSoftLight(x + 1, y + 1, z + 1, {1, 0, 0}, {0, 1, 0}),
				pickSoftLight(x, y + 1, z + 1, {1, 0, 0}, {0, 1, 0}) };
		face(glm::vec3(x, y, z), size.x, size.y, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), texfaces[5], lights, do_tint(0.9f), rot == 1);
	}
	if (isOpen(x, y, z - 1, group)) {
		glm::vec4 lights[]{
				pickSoftLight(x, y, z - 1, {-1, 0, 0}, {0, 1, 0}),
				pickSoftLight(x - 1, y, z - 1, {-1, 0, 0}, {0, 1, 0}),
				pickSoftLight(x - 1, y + 1, z - 1, {-1, 0, 0}, {0, 1, 0}),
				pickSoftLight(x, y + 1, z - 1, {-1, 0, 0}, {0, 1, 0}) };
		face(glm::vec3(x + size.x, y, z - size.z), size.x, size.y, glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), texfaces[4], lights, do_tint(0.75f), rot == 1);
	}

	if (isOpen(x, y + 1, z, group)) {
		glm::vec4 lights[]{
				pickSoftLight(x, y + 1, z + 1, {1, 0, 0}, {0, 0, 1}),
				pickSoftLight(x + 1, y + 1, z + 1, {1, 0, 0}, {0, 0, 1}),
				pickSoftLight(x + 1, y + 1, z, {1, 0, 0}, {0, 0, 1}),
				pickSoftLight(x, y + 1, z, {1, 0, 0}, {0, 0, 1}) };

		face(glm::vec3(x, y + size.y, z), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), texfaces[3], lights, glm::vec4(1.0f), rot == 1);
	}

	if (isOpen(x, y - 1, z, group)) {
		glm::vec4 lights[]{
				pickSoftLight(x, y - 1, z - 1, {1, 0, 0}, {0, 0, -1}),
				pickSoftLight(x + 1, y - 1, z - 1, {1, 0, 0}, {0, 0,-1}),
				pickSoftLight(x + 1, y - 1, z, {1, 0, 0}, {0, 0, -1}),
				pickSoftLight(x, y - 1, z, {1, 0, 0}, {0, 0, -1}) };
		face(glm::vec3(x, y, z - size.z), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), texfaces[2], lights, do_tint(0.6f), rot == 1);
	}

	if (isOpen(x - 1, y, z, group)) {
		glm::vec4 lights[]{
				pickSoftLight(x - 1, y, z - 1, {0, 0, -1}, {0, 1, 0}),
				pickSoftLight(x - 1, y, z, {0, 0, -1}, {0, 1, 0}),
				pickSoftLight(x - 1, y + 1, z, {0, 0, -1}, {0, 1, 0}),
				pickSoftLight(x - 1, y + 1, z - 1, {0, 0, -1}, {0, 1, 0}) };
		face(glm::vec3(x, y, z - size.z), size.z, size.y, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), texfaces[0], lights, do_tint(0.7f), rot == 3);
	}
	if (isOpen(x + 1, y, z, group)) {
		glm::vec4 lights[]{
				pickSoftLight(x + 1, y, z, {0, 0, -1}, {0, 1, 0}),
				pickSoftLight(x + 1, y, z - 1, {0, 0, -1}, {0, 1, 0}),
				pickSoftLight(x + 1, y + 1, z - 1, {0, 0, -1}, {0, 1, 0}),
				pickSoftLight(x + 1, y + 1, z, {0, 0, -1}, {0, 1, 0}) };
		face(glm::vec3(x + size.x, y, z), size.z, size.y, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), texfaces[1], lights, do_tint(0.8f), rot == 3);
	}
}

// Does block allow to see other blocks sides (is it transparent)
bool BlocksRenderer::isOpen(int x, int y, int z, ubyte group) const {
	blockid_t id = voxelsBuffer->pickBlockId(chunk->chunk_x * CHUNK_WIDTH + x, y, chunk->chunk_z * CHUNK_DEPTH + z);
	if (id == BLOCK_VOID) return false;
	const Block& block = *Block::blocks[id];
	if (block.drawGroup != group && block.lightPassing) return true;
	return id == Blocks_id::AIR;
}

bool BlocksRenderer::isOpenForLight(int x, int y, int z) const {
	blockid_t id = voxelsBuffer->pickBlockId(chunk->chunk_x * CHUNK_WIDTH + x, y, chunk->chunk_z * CHUNK_DEPTH + z);
	if (id == BLOCK_VOID) return false;
	const Block& block = *Block::blocks[id];
	if (block.lightPassing) return true;
	return id == Blocks_id::AIR;
}

glm::vec4 BlocksRenderer::pickLight(int x, int y, int z) const {
	if (isOpenForLight(x, y, z)) {
		light_t light = voxelsBuffer->pickLight(chunk->chunk_x * CHUNK_WIDTH + x, y, chunk->chunk_z * CHUNK_DEPTH + z);
		return glm::vec4(LightMap::extract(light, 0) / 15.0f,
			LightMap::extract(light, 1) / 15.0f,
			LightMap::extract(light, 2) / 15.0f,
			LightMap::extract(light, 3) / 15.0f);
	} else {
		return glm::vec4(0.0f);
	}
}

glm::vec4 BlocksRenderer::pickSoftLight(int x, int y, int z, glm::ivec3 right, glm::ivec3 up) const {
	return (pickLight(x - right.x - up.x, y - right.y - up.y, z - right.z - up.z) +
		pickLight(x - up.x, y - up.y, z - up.z) +
		pickLight(x, y, z) +
		pickLight(x - right.x, y - right.y, z - right.z)) * 0.25f;
}

// Get texture atlas UV region for block face
inline UVRegion uvfor(const Block& def, uint face, int atlas_size) {
	float uvsize = 1.0f / (float)atlas_size;
	const uint id = def.textureFaces[face];
	float u = (id % atlas_size) * uvsize;
	float v = 1.0f - (id / atlas_size + 1) * uvsize;
	return UVRegion(u, v, u + uvsize, v + uvsize);
}

void BlocksRenderer::render(const voxel* voxels, int atlas_size) {
	for (ubyte group = 0; group < 8; group++) {
		for (uint y = 0; y < CHUNK_HEIGHT; y++) {
			for (uint z = 0; z < CHUNK_DEPTH; z++) {
				for (uint x = 0; x < CHUNK_WIDTH; x++) {
					const voxel& vox = voxels[((y * CHUNK_DEPTH) + z) * CHUNK_WIDTH + x];
					blockid_t id = vox.id;
					const Block& def = *Block::blocks[id];
					if (!id || def.drawGroup != group) continue;
					const UVRegion texfaces[6]{ uvfor(def, 0, atlas_size), uvfor(def, 1, atlas_size),
												uvfor(def, 2, atlas_size), uvfor(def, 3, atlas_size),
												uvfor(def, 4, atlas_size), uvfor(def, 5, atlas_size) };
					switch (def.model) {
					case Block_models::CUBE:
						if (*((light_t*)&def.emission)) {
							blockCube(x, y, z, glm::vec3(1, 1, 1), texfaces, def.drawGroup);
						}
						else {
							blockCubeShaded(x, y, z, glm::vec3(1, 1, 1), texfaces, &def, vox.states);
						}
						break;
					case Block_models::X: {
						blockXSprite(x, y, z, glm::vec3(1, 1, 1), texfaces[FACE_MX], texfaces[FACE_MZ], 1.0f);
						break;
					}
					}
					if (overflow) return;
				}
			}
		}
	}
}

Mesh* BlocksRenderer::render(const Chunk* chunk, int atlas_size, const ChunksStorage* chunks) {
	this->chunk = chunk;
	voxelsBuffer->setPosition(chunk->chunk_x * CHUNK_WIDTH - 1, 0, chunk->chunk_z * CHUNK_DEPTH - 1);
	chunks->getVoxels(voxelsBuffer);
	overflow = false;
	offset = 0;
	const voxel* voxels = chunk->voxels;
	render(voxels, atlas_size);

	Mesh* mesh = new Mesh(buffer, offset / BlocksRenderer_Consts::VERTEX_SIZE, BlocksRenderer_Consts::ATTRS);
	return mesh;
}

VoxelsVolume* BlocksRenderer::getVoxelsBuffer() const {
	return voxelsBuffer;
}
