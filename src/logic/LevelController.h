#ifndef LOGIC_LEVEL_CONTROLLER_H_
#define LOGIC_LEVEL_CONTROLLER_H_

#include <memory>

#include "PlayerController.h"
#include "BlocksController.h"
#include "ChunksController.h"
#include "../settings.h"

class Level;

class LevelController {
private:
    EngineSettings& settings;

    std::unique_ptr<Level> level;

    std::unique_ptr<ChunksController> chunks;
    std::unique_ptr<PlayerController> player;
    std::unique_ptr<BlocksController> blocks;
public:
    LevelController(EngineSettings& settings, Level* level);

    void update(float delta, bool input, bool pause);

    void onWorldSave();
    void onWorldQuit();

    Level* getLevel();
    Player* getPlayer();

    PlayerController* getPlayerController();
};

#endif // LOGIC_LEVEL_CONTROLLER_H_
