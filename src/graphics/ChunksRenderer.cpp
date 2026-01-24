#include "ChunksRenderer.h"

#include "Mesh.h"
#include "BlocksRenderer.h"
#include "../voxels/Chunk.h"
#include "../world/Level.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

ChunksRenderer::ChunksRenderer(Level* level) : level(level) {
	const int MAX_FULL_CUBES = 3000;
	renderer = new BlocksRenderer(9 * 6 * 6 * MAX_FULL_CUBES);
}

ChunksRenderer::~ChunksRenderer() {
	delete renderer;
}

std::shared_ptr<Mesh> ChunksRenderer::render(Chunk* chunk) {
	chunk->setModified(false);
	Mesh* mesh = renderer->render(chunk, 16, level->chunksStorage);
	auto sptr = std::shared_ptr<Mesh>(mesh);
	meshes[glm::ivec2(chunk->chunk_x, chunk->chunk_z)] = sptr;
	return sptr;
}

void ChunksRenderer::unload(Chunk* chunk) {
	auto found = meshes.find(glm::ivec2(chunk->chunk_x, chunk->chunk_z));
	if (found != meshes.end()) meshes.erase(found);
}

std::shared_ptr<Mesh> ChunksRenderer::getOrRender(Chunk* chunk) {
	auto found = meshes.find(glm::ivec2(chunk->chunk_x, chunk->chunk_z));
	if (found != meshes.end() && !chunk->isModified()) return found->second;

	return render(chunk);
}

std::shared_ptr<Mesh> ChunksRenderer::get(Chunk* chunk) {
	auto found = meshes.find(glm::ivec2(chunk->chunk_x, chunk->chunk_z));
	if (found != meshes.end()) return found->second;

	return nullptr;
}
