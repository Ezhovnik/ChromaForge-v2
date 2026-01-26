#ifndef SRC_DECLARATIONS_H_
#define SRC_DECLARATIONS_H_

#include "typedefs.h"

namespace BlockID {
    inline constexpr blockid_t AIR = 0;
    inline constexpr blockid_t MOSS = 1;
    inline constexpr blockid_t DIRT = 2;
    inline constexpr blockid_t STONE = 3;
    inline constexpr blockid_t COBBLESTONE = 4;
    inline constexpr blockid_t GLOWSTONE = 5;
    inline constexpr blockid_t GLASS = 6;
    inline constexpr blockid_t PLANKS = 7;
    inline constexpr blockid_t LOG = 8;
    inline constexpr blockid_t LEAVES = 9;
    inline constexpr blockid_t WATER = 10;
    inline constexpr blockid_t SAND = 11;
    inline constexpr blockid_t BEDROCK = 12;
    inline constexpr blockid_t BRICKS = 13;
    inline constexpr blockid_t GRASS = 14;
    inline constexpr blockid_t POPPY = 15;
    inline constexpr blockid_t DANDELION = 16;
    inline constexpr blockid_t DAISY = 17;
    inline constexpr blockid_t MARIGOLD = 18;
    inline constexpr blockid_t RED_NEON = 19;
    inline constexpr blockid_t GREEN_NEON = 20;
    inline constexpr blockid_t BLUE_NEON = 21;
}

class AssetsLoader;

void initialize_assets(AssetsLoader* loader);
void setup_definitions();

#endif // SRC_DECLARATIONS_H_
