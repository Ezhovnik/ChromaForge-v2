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

// Точка спавна игрока и начальная скорость
inline constexpr glm::vec3 SPAWNPOINT = {0, 256, 0}; // Точка, где игрок появляется в мире
inline constexpr float DEFAULT_PLAYER_SPEED = 5.0f; // Начальная скорость перемещения игрока

World::World(std::string name, std::filesystem::path directory, uint64_t seed, EngineSettings& settings) : name(name), seed(seed) {
	wfile = new WorldFiles(directory, settings.debug.generatorTestMode);
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

	for (size_t i = 0; i < chunks->volume; i++) {
		std::shared_ptr<Chunk> chunk = chunks->chunks[i];
		if (chunk == nullptr || !chunk->isUnsaved()) continue;
		wfile->put(chunk.get());
	}

	wfile->write(this, content);
	wfile->writePlayer(level->player);
}

Level* World::load(EngineSettings& settings, const Content* content) {
    Camera* camera = new Camera(SPAWNPOINT, glm::radians(90.0f));
    Player* player = new Player(SPAWNPOINT, DEFAULT_PLAYER_SPEED, camera);

    LOG_INFO("Reading info about the world");
	wfile->readWorldInfo(this);
    LOG_INFO("Info about the world has been successfully read");

    LOG_INFO("Creating a level");
	Level* level = new Level(this, content, player, settings);

    LOG_INFO("Reading player info");
	wfile->readPlayer(player);
    LOG_INFO("Player info successfully read");

	camera->rotation = glm::mat4(1.0f);
	camera->rotate(player->camY, player->camX, 0);

    LOG_INFO("Level successfully created");
    Logger::getInstance().flush();
	return level;
}
