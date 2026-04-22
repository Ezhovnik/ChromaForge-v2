#pragma once

#include <memory>

#include <logic/PlayerController.h>
#include <logic/BlocksController.h>
#include <logic/ChunksController.h>

class Level;
struct EngineSettings;

class LevelController {
private:
    EngineSettings& settings;

    std::unique_ptr<Level> level;

    std::unique_ptr<BlocksController> blocks;
    std::unique_ptr<ChunksController> chunks;
    std::unique_ptr<PlayerController> player;
public:
    LevelController(
        EngineSettings& settings,
        std::unique_ptr<Level> level
    );

    void update(float delta, bool input, bool pause);

    void saveWorld();
    void onWorldQuit();

    Level* getLevel();
    Player* getPlayer();

    PlayerController* getPlayerController();
    BlocksController* getBlocksController();
};
