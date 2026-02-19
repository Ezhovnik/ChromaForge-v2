#include "World.h"

#include <memory>

#include "Level.h"
#include "LevelEvents.h"
#include "../files/WorldFiles.h"
#include "../voxels/Chunk.h"
#include "../voxels/Chunks.h"
#include "../voxels/ChunksStorage.h"
#include "../objects/Player.h"
#include "../physics/PhysicsSolver.h"
#include "../window/Camera.h"
#include "../logger/Logger.h"
#include "../content/Content.h"
#include "../content/ContentLUT.h"

// Точка спавна игрока и начальная скорость
inline constexpr glm::vec3 SPAWNPOINT = {0, 256, 0}; // Точка, где игрок появляется в мире
inline constexpr float DEFAULT_PLAYER_SPEED = 5.0f; // Начальная скорость перемещения игрока

world_load_error::world_load_error(std::string message) : std::runtime_error(message) {
}

World::World(std::string name, std::filesystem::path directory, uint64_t seed, EngineSettings& settings, const Content* content) : name(name), seed(seed), settings(settings), content(content) {
	wfile = new WorldFiles(directory, settings.debug);
}

World::~World(){
	delete wfile;
}

void World::updateTimers(float delta) {
	daytime += delta * daytimeSpeed;
	daytime = fmod(daytime, 1.0f);
}

void World::write(Level* level) {
	const Content* content = level->content;
	Chunks* chunks = level->chunks;

	for (size_t i = 0; i < chunks->volume; ++i) {
		std::shared_ptr<Chunk> chunk = chunks->chunks[i];
		if (chunk == nullptr || !chunk->isLighted()) continue;
		bool lightsUnsaved = !chunk->isLoadedLights() && settings.debug.doWriteLights;
		if (!chunk->isUnsaved() && !lightsUnsaved) continue;
		wfile->put(chunk.get());
	}

	wfile->write(this, content);
	wfile->writePlayer(level->player);
}

Level* World::create(std::string name, std::filesystem::path directory, uint64_t seed, EngineSettings& settings, const Content* content) {
	LOG_INFO("Creating world");
	World* world = new World(name, directory, seed, settings, content);
	LOG_INFO("World successfully created");

	LOG_INFO("Creating a level");
	Player* player = new Player(SPAWNPOINT, DEFAULT_PLAYER_SPEED);
	Level* level = new Level(world, content, player, settings);
	LOG_INFO("Level successfully created");

	Logger::getInstance().flush();

	return level;
}

ContentLUT* World::checkIndices(const std::filesystem::path& directory, const Content* content) {
	std::filesystem::path indicesFile = directory/std::filesystem::path("indices.json");
	if (std::filesystem::is_regular_file(indicesFile)) return ContentLUT::create(indicesFile, content);

	return nullptr;
}

Level* World::load(std::filesystem::path directory, EngineSettings& settings, const Content* content) {
	LOG_INFO("Loading world");
	std::unique_ptr<World> world (new World(".", directory, 0, settings, content));
	auto& wfile = world->wfile;

	if (!wfile->readWorldInfo(world.get())) {
		LOG_ERROR("Could not to find world.json");
		Logger::getInstance().flush();
		throw world_load_error("Could not to find world.json");
	}

	LOG_INFO("Creating a level");
	Player* player = new Player(SPAWNPOINT, DEFAULT_PLAYER_SPEED);
	wfile->readPlayer(player);
	Level* level = new Level(world.get(), content, player, settings);
	LOG_INFO("Level successfully created");

	world.release();
	LOG_INFO("World successfully created");
	return level;
}
