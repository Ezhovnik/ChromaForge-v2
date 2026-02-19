#ifndef WORLD_WORLD_H_
#define WORLD_WORLD_H_

#include <string>
#include <filesystem>
#include <stdexcept>

#include "../typedefs.h"
#include "../settings.h"
#include "../util/timeutil.h"

class world_load_error : public std::runtime_error {
public:
	world_load_error(std::string message);
};

class WorldFiles;
class Level;
class Player;
class Content;
class ContentLUT;

class World {
private:
	EngineSettings& settings;
	const Content* const content;
public:
	std::string name;
	WorldFiles* wfile;
	uint64_t seed;

	float daytime = timeutil::time_value(10, 00, 00);
	float daytimeSpeed = 1.0f/60.0f/24.0f;

	World(std::string name, std::filesystem::path directory, uint64_t seed, EngineSettings& settings, const Content* content);
	~World();

	void updateTimers(float delta);

    void write(Level* level);

	static ContentLUT* checkIndices(const std::filesystem::path& directory, const Content* content);

	static Level* create(std::string name, std::filesystem::path directory, uint64_t seed, EngineSettings& settings, const Content* content);
    static Level* load(std::filesystem::path directory, EngineSettings& settings, const Content* content);
};

#endif // WORLD_WORLD_H_
