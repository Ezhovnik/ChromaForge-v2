#pragma once

#include <functional>

#include <glm/glm.hpp>

#include <math/rand.h>
#include <typedefs.h>
#include <voxels/voxel.h>
#include <util/Clock.h>

class Player;
class Block;
class Level;
class Chunks;
class Lighting;
class Chunk;
class ContentIndices;
class GlobalChunks;

enum class BlockInteraction {
    Step,
    Destruction,
    Placing
};

using on_block_interaction = std::function<void(
    Player*, const glm::ivec3&, const Block&, BlockInteraction
)>;

class BlocksController {
    const Level& level;
	GlobalChunks& chunks;
    Lighting* lighting;
    util::Clock randSparkClock;
    util::Clock blocksSparkClock;
    util::Clock worldSparkClock;

    FastRandom random {};

    std::vector<on_block_interaction> blockInteractionCallbacks;
public:
    BlocksController(const Level& level, Lighting* lighting);

    void updateSides(int x, int y, int z);
    void updateSides(int x, int y, int z, int w, int h, int d);
    void updateBlock(int x, int y, int z);

    void breakBlock(
        Player* player,
        const Block& def,
        int x, int y, int z
    );
    void placeBlock(
        Player* player,
        const Block& def,
        blockstate state,
        int x, int y, int z
    );

    void update(float delta, uint padding);
    void randomSpark(
        const Chunk& chunk,
        int segments,
        const ContentIndices* indices
    );
    void randomSpark(int sparkId, int parts, uint padding);
    void onBlocksSpark(int sparkId, int parts);

    int64_t createBlockInventory(int x, int y, int z);
    void bindInventory(int64_t invId, int x, int y, int z);
    void unbindInventory(int x, int y, int z);

    void onBlockInteraction(
        Player* player,
        glm::ivec3 pos,
        const Block& def,
        BlockInteraction type
    );

    void listenBlockInteraction(const on_block_interaction& callback);
};
