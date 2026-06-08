#pragma once

#include <memory>
#include <unordered_map>
#include <mutex>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <typedefs.h>

class Chunk;
class Level;
class ContentIndices;

class ChunksStorage {
private:
    Level* level;
	std::mutex mutex;
    std::unordered_map<glm::ivec2, std::weak_ptr<Chunk>> chunksMap;
public:
	ChunksStorage(Level* level);
	~ChunksStorage() = default;

	std::shared_ptr<Chunk> fetch(int x, int z);
    std::shared_ptr<Chunk> create(int x, int z);

	light_t getLight(int x, int y, int z, ubyte channel) const;
};
