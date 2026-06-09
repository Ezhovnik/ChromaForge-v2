#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <typedefs.h>
#include <util/WeakPtrsMap.h>

class Chunk;
class Level;
class ContentIndices;

class ChunksStorage {
private:
    Level* level;
	std::shared_ptr<util::WeakPtrsMap<glm::ivec2, Chunk>> chunksMap;
public:
	ChunksStorage(Level* level);
	~ChunksStorage() = default;

	std::shared_ptr<Chunk> fetch(int x, int z);
    std::shared_ptr<Chunk> create(int x, int z);

	light_t getLight(int x, int y, int z, ubyte channel) const;
};
