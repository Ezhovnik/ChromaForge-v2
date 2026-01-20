#ifndef WORLD_LEVEL_H_
#define WORLD_LEVEL_H_

#include "../typedefs.h"

class World;
class Player;
class Chunks;
class Lighting;
class PhysicsSolver;
class ChunksController;
class PlayerController;
class ChunksStorage;
class LevelEvents;

class Level {
public:
	World* world;
	Player* player;
	Chunks* chunks;
	PhysicsSolver* physics;
	Lighting* lighting;
	ChunksController* chunksController;
    ChunksStorage* chunksStorage;
	PlayerController* playerController;
    LevelEvents* events;

	Level(World* world, Player* player, ChunksStorage* storage, LevelEvents* events, uint loadDistance, uint chunksPadding);
	~Level();

    void update(float deltaTime, bool interactions);
};

#endif // WORLD_LEVEL_H_
