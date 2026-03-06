#ifndef WORLD_WORLD_H_
#define WORLD_WORLD_H_

#include <string>
#include <filesystem>
#include <stdexcept>

#include "../typedefs.h"
#include "../settings.h"
#include "../util/timeutil.h"
#include "../content/ContentPack.h"

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
	std::vector<ContentPack> packs;

	uint64_t seed;
	std::string name;
public:
	WorldFiles* wfile;

	float daytime = timeutil::time_value(10, 00, 00);
	float daytimeSpeed = 1.0f / 60.0f / 24.0f;
	double totalTime = 0.0;

	World(std::string name, std::filesystem::path directory, uint64_t seed, EngineSettings& settings, const Content* content, std::vector<ContentPack> packs);
	~World();

	void updateTimers(float delta);

    void write(Level* level);

	static ContentLUT* checkIndices(const std::filesystem::path& directory, const Content* content);

	static Level* create(std::string name, std::filesystem::path directory, uint64_t seed, EngineSettings& settings, const Content* content, const std::vector<ContentPack>& packs);
    static Level* load(std::filesystem::path directory, EngineSettings& settings, const Content* content, const std::vector<ContentPack>& packs);

	void setName(const std::string& name);
    void setSeed(uint64_t seed);

    std::string getName() const;
    uint64_t getSeed() const;

	const std::vector<ContentPack>& getPacks() const;
};

#endif // WORLD_WORLD_H_
