#ifndef VOXELS_VOXEL_H_
#define VOXELS_VOXEL_H_

#include <stdint.h>
#include "../typedefs.h"

constexpr int BLOCK_DIR_NORTH = 0x0;
constexpr int BLOCK_DIR_WEST = 0x1;
constexpr int BLOCK_DIR_SOUTH = 0x2;
constexpr int BLOCK_DIR_EAST = 0x3;
constexpr int BLOCK_DIR_UP = 0x4;
constexpr int BLOCK_DIR_DOWN = 0x5;

constexpr int BLOCK_ROT_MASK = 0xF;

// Структура, представляющая один воксель
struct voxel {
    blockid_t id;
    uint8_t states;
};

#endif // VOXELS_VOXEL_H_
