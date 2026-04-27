#pragma once

#include <memory>

#include <typedefs.h>

class Level;
class Chunks;
class Lighting;
class WorldFiles;
class VoxelRenderer;
class ChunksLoader;
class WorldGenerator;
class Chunk;

class ChunksController {
private:
	Level* level;
	Chunks* chunks;
	Lighting* lighting;
	std::unique_ptr<WorldGenerator> generator;

    uint chunksPadding;

	bool loadVisible();
	bool buildLights(const std::shared_ptr<Chunk>& chunk);
    void createChunk(int x, int y);
public:
	ChunksController(Level* level, uint chunksPadding);
	~ChunksController();

    void update(
        int64_t maxDuration,
        int loadDistance,
        int centerX,
        int centerY
	);
};
