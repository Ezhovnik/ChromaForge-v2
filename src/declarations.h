#ifndef SRC_DECLARATIONS_H_
#define SRC_DECLARATIONS_H_

namespace Blocks_id {
    inline constexpr int AIR = 0;
    inline constexpr int MOSS = 1;
    inline constexpr int DIRT = 2;
    inline constexpr int STONE = 3;
    inline constexpr int GLOWSTONE = 4;
    inline constexpr int GLASS = 5;
    inline constexpr int PLANKS = 6;
    inline constexpr int LOG = 7;
    inline constexpr int LEAVES = 8;
    inline constexpr int WATER = 9;
    inline constexpr int SAND = 10;
    inline constexpr int BEDROCK = 11;
    inline constexpr int BRICKS = 12;
    inline constexpr int GRASS = 13;
    inline constexpr int POPPY = 14;
    inline constexpr int DANDELION = 15;
    inline constexpr int DAISY = 16;
}

class AssetsLoader;

void initialize_assets(AssetsLoader* loader);
void setup_definitions();

#endif // SRC_DECLARATIONS_H_
