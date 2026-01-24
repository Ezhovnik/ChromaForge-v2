#ifndef WORLD_WORLD_H_
#define WORLD_WORLD_H_

#include <filesystem>
#include <string>

#include "../typedefs.h"
#include "../settings.h"

class WorldFiles;
class Level;
class Player;

class World {
public:
	std::string name;
	WorldFiles* wfile;
	uint64_t seed;

	World(std::string name, std::filesystem::path directory, uint64_t seed, EngineSettings& settings);
	~World();

    void write(Level* level, bool writeChunks);
    Level* loadLevel(EngineSettings& settings);
};

#endif // WORLD_WORLD_H_
