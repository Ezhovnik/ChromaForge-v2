#ifndef WORLD_LEVEL_H_
#define WORLD_LEVEL_H_

#include <memory>

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

class Level {
public:
	std::unique_ptr<World> world;
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

	World* getWorld();
};

#endif // WORLD_LEVEL_H_
