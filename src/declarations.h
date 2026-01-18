#ifndef SRC_DECLARATIONS_H_
#define SRC_DECLARATIONS_H_

#include "typedefs.h"

namespace Blocks_id {
    inline constexpr blockid_t AIR = 0;
    inline constexpr blockid_t MOSS = 1;
    inline constexpr blockid_t DIRT = 2;
    inline constexpr blockid_t STONE = 3;
    inline constexpr blockid_t GLOWSTONE = 4;
    inline constexpr blockid_t GLASS = 5;
    inline constexpr blockid_t PLANKS = 6;
    inline constexpr blockid_t LOG = 7;
    inline constexpr blockid_t LEAVES = 8;
    inline constexpr blockid_t WATER = 9;
    inline constexpr blockid_t SAND = 10;
    inline constexpr blockid_t BEDROCK = 11;
    inline constexpr blockid_t BRICKS = 12;
    inline constexpr blockid_t GRASS = 13;
    inline constexpr blockid_t POPPY = 14;
    inline constexpr blockid_t DANDELION = 15;
    inline constexpr blockid_t DAISY = 16;
    inline constexpr blockid_t RED_NEON = 17;
    inline constexpr blockid_t GREEN_NEON = 18;
    inline constexpr blockid_t BLUE_NEON = 19;

}

class AssetsLoader;

void initialize_assets(AssetsLoader* loader);
void setup_definitions();

#endif // SRC_DECLARATIONS_H_
