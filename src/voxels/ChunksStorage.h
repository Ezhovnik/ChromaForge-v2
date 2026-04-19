#ifndef VOXELS_CHUNKSSTORAGE_H_
#define VOXELS_CHUNKSSTORAGE_H_

#include <memory>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <voxels/voxel.h>
#include <typedefs.h>

class Chunk;
class VoxelsVolume;
class Level;
class ContentIndices;

class ChunksStorage {
private:
    Level* level;
	std::unordered_map<glm::ivec2, std::shared_ptr<Chunk>> chunksMap;

	void verifyLoadedChunk(ContentIndices* indices, Chunk* chunk);
public:
	ChunksStorage(Level* level);
	~ChunksStorage() = default;

	std::shared_ptr<Chunk> get(int x, int z) const;
	void store(const std::shared_ptr<Chunk>& chunk);
    void remove(int x, int z);
	void getVoxels(VoxelsVolume* volume, bool backlight = false) const;
    std::shared_ptr<Chunk> create(int x, int z);

	light_t getLight(int x, int y, int z, ubyte channel) const;
};


#endif // VOXELS_CHUNKSSTORAGE_H_
