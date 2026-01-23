#include "World.h"

#include "../files/WorldFiles.h"
#include "../voxels/Chunk.h"
#include "../voxels/Chunks.h"
#include "../voxels/ChunksStorage.h"
#include "Level.h"
#include "LevelEvents.h"
#include "../objects/Player.h"
#include "../physics/PhysicsSolver.h"
#include "../window/Camera.h"

World::World(std::string name, std::string directory, int seed, EngineSettings& settings) : name(name), seed(seed) {
	wfile = new WorldFiles(directory, Region_Consts::REGION_VOLUME * (CHUNK_DATA_LEN * 2 + 8), settings.debug.generatorTestMode);
}

World::~World(){
	delete wfile;
}

void World::write(Level* level, bool writeChunks) {
	Chunks* chunks = level->chunks;

	for (size_t i = 0; i < chunks->volume; ++i) {
		std::shared_ptr<Chunk> chunk = chunks->chunks[i];
		if (chunk == nullptr || !chunk->isUnsaved()) continue;
        wfile->put(chunk.get());
	}

	wfile->write();
	wfile->writePlayer(level->player);
}

Level* World::loadLevel(Player* player, EngineSettings& settings) {
    ChunksStorage* storage = new ChunksStorage();
    LevelEvents* events = new LevelEvents();
	Level* level = new Level(this, player, storage, events, settings);
	wfile->readPlayer(player);

	Camera* camera = player->camera;
	camera->rotation = glm::mat4(1.0f);
	camera->rotate(player->camY, player->camX, 0);
	return level;
}
