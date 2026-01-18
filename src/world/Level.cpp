#include "Level.h"

#include "../lighting/Lighting.h"
#include "../voxels/ChunksController.h"
#include "../voxels/Chunks.h"
#include "../physics/PhysicsSolver.h"
#include "../voxels/ChunksStorage.h"
#include "../physics/Hitbox.h"
#include "../objects/Player.h"
#include "../player_control.h"
#include "World.h"

Level::Level(World* world, Player* player, Chunks* chunks, ChunksStorage* chunksStorage, PhysicsSolver* physics) :
	world(world),
	player(player),
	chunks(chunks),
    chunksStorage(chunksStorage),
	physics(physics) {
	lighting = new Lighting(chunks);
	chunksController = new ChunksController(this, chunks, lighting);
	playerController = new PlayerController(this);
}

Level::~Level(){
	delete chunks;
	delete physics;
	delete player;
	delete lighting;
	delete chunksController;
    delete chunksStorage;
	delete playerController;
}

void Level::update(float deltaTime, bool interactions) {
	playerController->update_controls(deltaTime);
	if (interactions) {
		playerController->update_interaction();
	} else {
		playerController->selectedBlockId = -1;
	}
	glm::vec3 position = player->hitbox->position;
	chunks->setCenter(world->wfile, position.x, position.z);
}
