#ifndef LOGIC_LEVEL_CONTROLLER_H_
#define LOGIC_LEVEL_CONTROLLER_H_

#include <memory>

#include "../settings.h"

class Level;
class ChunksController;
class PlayerController;
class BlocksController;

class LevelController {
    EngineSettings& settings;

    Level* level;

    std::unique_ptr<ChunksController> chunks;
    std::unique_ptr<PlayerController> player;
    std::unique_ptr<BlocksController> blocks;
public:
    LevelController(EngineSettings& settings, Level* level);
    ~LevelController();

    void update(float delta, bool input, bool pause);
};

#endif // LOGIC_LEVEL_CONTROLLER_H_
