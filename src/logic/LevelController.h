#pragma once

#include <memory>

#include <logic/BlocksController.h>
#include <logic/ChunksController.h>
#include <util/Clock.h>

class Level;
struct EngineSettings;
class Engine;

class LevelController {
private:
    EngineSettings& settings;

    std::unique_ptr<Level> level;

    std::unique_ptr<BlocksController> blocks;
    std::unique_ptr<ChunksController> chunks;

    util::Clock playerSparkClock;
public:
    LevelController(
        Engine* engine,
        std::unique_ptr<Level> level,
        Player* clientPlayer
    );

    void update(float delta, bool pause);

    void saveWorld();
    void onWorldQuit();

    Level* getLevel();

    ChunksController* getChunksController();
    BlocksController* getBlocksController();
};
