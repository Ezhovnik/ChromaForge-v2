#ifndef LOGIC_BLOCKS_CONTROLLER_H_
#define LOGIC_BLOCKS_CONTROLLER_H_

#include <functional>

#include <glm/glm.hpp>

#include "math/rand.h"
#include "typedefs.h"
#include "voxels/voxel.h"
#include "util/Clock.h"

class Player;
class Block;
class Level;
class Chunks;
class Lighting;
class Chunk;
class ContentIndices;

enum class BlockInteraction {
    Step,
    Destruction,
    Placing
};

using on_block_interaction = std::function<void(
    Player*, glm::ivec3, const Block*, BlockInteraction type
)>;

class BlocksController {
    Level* level;
	Chunks* chunks;
	Lighting* lighting;
    util::Clock randSparkClock;
    util::Clock blocksSparkClock;
    util::Clock worldSparkClock;
    uint padding;

    FastRandom random;

    std::vector<on_block_interaction> blockInteractionCallbacks;
public:
    BlocksController(Level* level, uint padding);

    void updateSides(int x, int y, int z);
    void updateBlock(int x, int y, int z);

    void breakBlock(
        Player* player,
        const Block* def,
        int x, int y, int z
    );
    void placeBlock(
        Player* player,
        const Block* def,
        blockstate state,
        int x, int y, int z
    );

    void update(float delta);
    void randomSpark(
        const Chunk& chunk,
        int segments,
        const ContentIndices* indices
    );
    void randomSpark(int sparkId, int parts);
    void onBlocksSpark(int sparkId, int parts);

    int64_t createBlockInventory(int x, int y, int z);
    void bindInventory(int64_t invId, int x, int y, int z);
    void unbindInventory(int x, int y, int z);

    void onBlockInteraction(
        Player* player,
        glm::ivec3 pos,
        const Block* def,
        BlockInteraction type
    );

    void listenBlockInteraction(const on_block_interaction& callback);
};

#endif // LOGIC_BLOCKS_CONTROLLER_H_
