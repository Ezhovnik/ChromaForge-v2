#include "Level.h"

#include "../lighting/Lighting.h"
#include "../voxels/Chunks.h"
#include "../voxels/ChunksStorage.h"
#include "../voxels/Chunk.h"
#include "../physics/PhysicsSolver.h"
#include "../physics/Hitbox.h"
#include "../objects/Player.h"
#include "World.h"
#include "LevelEvents.h"
#include "../content/Content.h"

inline constexpr float GRAVITY = 19.6f;

Level::Level(World* world, const Content* content, Player* player, EngineSettings& settings) :
	world(world),
	player(player),
    content(content),
    chunksStorage(new ChunksStorage(this)),
    events(new LevelEvents()) ,
    settings(settings)
{
    physics = new PhysicsSolver(glm::vec3(0, -GRAVITY, 0));
    uint matrixSize = (settings.chunks.loadDistance + settings.chunks.padding) * 2;
    chunks = new Chunks(matrixSize, matrixSize, 0, 0, world->wfile, events, content);
	lighting = new Lighting(content, chunks);
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
    delete chunksStorage;
}

void Level::update() {
	glm::vec3 position = player->hitbox->position;
	chunks->setCenter(position.x, position.z);

    int matrixSize = (settings.chunks.loadDistance + settings.chunks.padding) * 2;
    if (chunks->width != matrixSize) {
        chunks->resize(matrixSize, matrixSize);
    }
}
