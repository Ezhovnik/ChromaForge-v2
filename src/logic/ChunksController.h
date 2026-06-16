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
class Player;

class ChunksController {
private:
	Level& level;
	std::unique_ptr<WorldGenerator> generator;

	bool loadVisible(const Player& player, uint padding) const;
    bool buildLights(const Player& player, const std::shared_ptr<Chunk>& chunk) const;
    void createChunk(const Player& player, int x, int y) const;
public:
	std::unique_ptr<Lighting> lighting;

	ChunksController(Level& level);
	~ChunksController();

    void update(
        int64_t maxDuration,
        int loadDistance,
		uint padding,
        Player& player
	) const;

	const WorldGenerator* getGenerator() const {
		return generator.get();
	}
};
