#ifndef VOXELS_VOXEL_H_
#define VOXELS_VOXEL_H_

#include <stdint.h>
#include "../typedefs.h"

// Структура, представляющая один воксель
struct voxel {
    blockid_t id;
    uint8_t states;
};

#endif // VOXELS_VOXEL_H_
