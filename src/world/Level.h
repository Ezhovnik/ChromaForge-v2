#ifndef WORLD_LEVEL_H_
#define WORLD_LEVEL_H_

#include "../typedefs.h"
#include "../settings.h"

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
	PlayerController* playerController;
    ChunksStorage* chunksStorage;
    LevelEvents* events;
    const EngineSettings& settings;

	Level(World* world, Player* player, EngineSettings& settings);
	~Level();

    void updatePlayer(float deltaTime, bool input, bool pause, bool interactions);
    void update();
};

#endif // WORLD_LEVEL_H_
