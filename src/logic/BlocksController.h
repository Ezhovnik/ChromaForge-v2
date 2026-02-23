#ifndef LOGIC_BLOCKS_CONTROLLER_H_
#define LOGIC_BLOCKS_CONTROLLER_H_

#include "../typedefs.h"

class Player;
class Block;
class Level;
class Chunks;
class Lighting;

class Clock {
    int sparkRate;
    int sparkParts;

    float sparkTimer = 0.0f;
    int sparkId = 0;
    int sparkPartsUndone = 0;
public:
    Clock(int sparkRate, int sparkParts);

    bool update(float delta);

    int getParts() const;
    int getPart() const;
};

class BlocksController {
    Level* level;
	Chunks* chunks;
	Lighting* lighting;
    Clock randSparkClock;
    uint padding;
public:
    BlocksController(Level* level, uint padding);

    void updateSides(int x, int y, int z);
    void updateBlock(int x, int y, int z);

    void breakBlock(Player* player, const Block* def, int x, int y, int z);

    void update(float delta);
    void randomSpark(int sparkId, int parts);
};

#endif // LOGIC_BLOCKS_CONTROLLER_H_
