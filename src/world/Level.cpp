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

Level::Level(World* world, Player* player, Chunks* chunks, ChunksStorage* chunksStorage, PhysicsSolver* physics, LevelEvents* events) :
	world(world),
	player(player),
	chunks(chunks),
    chunksStorage(chunksStorage),
	physics(physics),
    events(events) {
	lighting = new Lighting(chunks);
	chunksController = new ChunksController(this, chunks, lighting);
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
	chunks->setCenter(world->wfile, position.x, position.z);
}
