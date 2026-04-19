#ifndef VOXELS_VOXEL_H_
#define VOXELS_VOXEL_H_

#include <stdint.h>

#include <typedefs.h>

/** Направление на север (вдоль отрицательной оси Z). */
inline constexpr int BLOCK_DIR_NORTH = 0x0;
/** Направление на запад (вдоль отрицательной оси X). */
inline constexpr int BLOCK_DIR_WEST = 0x1;
/** Направление на юг (вдоль положительной оси Z). */
inline constexpr int BLOCK_DIR_SOUTH = 0x2;
/** Направление на восток (вдоль положительной оси X). */
inline constexpr int BLOCK_DIR_EAST = 0x3;
/** Направление вверх (вдоль положительной оси Y). */
inline constexpr int BLOCK_DIR_UP = 0x4;
/** Направление вниз (вдоль отрицательной оси Y). */
inline constexpr int BLOCK_DIR_DOWN = 0x5;

struct blockstate {
    uint8_t rotation : 3;
    uint8_t segment : 3;
    uint8_t reserved : 2;
    uint8_t userbits : 8;
};
static_assert (sizeof(blockstate) == 2);

inline constexpr blockstate_t blockstate2int(blockstate b) {
    return static_cast<blockstate_t>(b.rotation) |
        static_cast<blockstate_t>(b.segment) << 3 |
        static_cast<blockstate_t>(b.reserved) << 6 |
        static_cast<blockstate_t>(b.userbits) << 8;
}

inline constexpr blockstate int2blockstate(blockstate_t i) {
    return {
        static_cast<uint8_t>(i & 0b111),
        static_cast<uint8_t>((i >> 3) & 0b111),
        static_cast<uint8_t>((i >> 6) & 0b11),
        static_cast<uint8_t>((i >> 8) & 0xFF)
    };
}

/**
 * @brief Структура, представляющая один воксель в мире.
 */
struct voxel {
    blockid_t id;
    blockstate state;
};
static_assert(sizeof(voxel) == 4);

#endif // VOXELS_VOXEL_H_
