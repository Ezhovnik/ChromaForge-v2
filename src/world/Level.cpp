#include "Level.h"

#include "../lighting/Lighting.h"
#include "../voxels/ChunksController.h"
#include "../voxels/Chunks.h"
#include "../physics/PhysicsSolver.h"
#include "../voxels/ChunksStorage.h"
#include "../voxels/Chunk.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../objects/player_control.h"
#include "World.h"
#include "LevelEvents.h"

inline constexpr float GRAVITY = 19.6f;

Level::Level(World* world, Player* player, ChunksStorage* chunksStorage, LevelEvents* events, uint loadDistance, uint chunksPadding) :
	world(world),
	player(player),
    chunksStorage(chunksStorage),
    events(events) {
    physics = new PhysicsSolver(glm::vec3(0, -GRAVITY, 0));

    uint matrixSize = (loadDistance + chunksPadding) * 2;
    chunks = new Chunks(matrixSize, matrixSize, 0, 0, world->wfile, events);

	lighting = new Lighting(chunks);
	chunksController = new ChunksController(this, chunks, lighting, chunksPadding);
	playerController = new PlayerController(this);
    events->listen(CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
		this->chunksStorage->remove(chunk->chunk_x, chunk->chunk_z);
	});
}

Level::~Level(){
	delete chunks;
	delete physics;
    delete events;
	delete player;
	delete lighting;
	delete chunksController;
    delete chunksStorage;
	delete playerController;
}

void Level::update(float deltaTime, bool interactions) {
	playerController->update_controls(deltaTime);
	if (interactions) playerController->update_interaction();
	else playerController->selectedBlockId = -1;

	glm::vec3 position = player->hitbox->position;
	chunks->setCenter(position.x, position.z);
}
