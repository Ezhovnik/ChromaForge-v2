#ifndef WORLD_LEVEL_H_
#define WORLD_LEVEL_H_

#include "../typedefs.h"
#include "../settings.h"

class World;
class Player;
class Chunks;
class Lighting;
class PhysicsSolver;
class ChunksStorage;
class LevelEvents;
class Content;
class ContentIndices;

class Level {
public:
	World* world;
	Player* player;
	Chunks* chunks;
	PhysicsSolver* physics;
	Lighting* lighting;
    ChunksStorage* chunksStorage;
    LevelEvents* events;
    const EngineSettings& settings;
    const Content* const content;

	Level(World* world, const Content* content, Player* player, EngineSettings& settings);
	~Level();

    void update();
};

#endif // WORLD_LEVEL_H_
