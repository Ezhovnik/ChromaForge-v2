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
#include "../typedefs.h"

inline constexpr float GRAVITY = 19.6f;

// FIXME: При дижении вдоль оси OZ чанки зацикливаются

World::World(std::string name, std::string directory, int seed) : name(name), seed(seed) {
	wfile = new WorldFiles(directory, Region_Consts::REGION_VOLUME * (CHUNK_DATA_LEN * 2 + 8));
}

World::~World(){
	delete wfile;
}

void World::write(Level* level) {
	Chunks* chunks = level->chunks;

	for (size_t i = 0; i < chunks->volume; ++i) {
		std::shared_ptr<Chunk> chunk = chunks->chunks[i];
		if (chunk == nullptr || !chunk->isUnsaved()) continue;
        wfile->put(chunk.get());
	}

	wfile->write();
	wfile->writePlayer(level->player);
}

Level* World::loadLevel(Player* player) {
    ChunksStorage* storage = new ChunksStorage();
    LevelEvents* events = new LevelEvents();
	Level* level = new Level(this, player, new Chunks(16, 16, 0, 0, events), storage, new PhysicsSolver(glm::vec3(0, -GRAVITY, 0)), events);
	wfile->readPlayer(player);

	Camera* camera = player->camera;
	camera->rotation = glm::mat4(1.0f);
	camera->rotate(player->camY, player->camX, 0);
	return level;
}
