#ifndef VOXELS_VOXEL_H_
#define VOXELS_VOXEL_H_

#include <stdint.h>
#include "../typedefs.h"

/** Направление на север (вдоль отрицательной оси Z). */
constexpr int BLOCK_DIR_NORTH = 0x0;
/** Направление на запад (вдоль отрицательной оси X). */
constexpr int BLOCK_DIR_WEST = 0x1;
/** Направление на юг (вдоль положительной оси Z). */
constexpr int BLOCK_DIR_SOUTH = 0x2;
/** Направление на восток (вдоль положительной оси X). */
constexpr int BLOCK_DIR_EAST = 0x3;
/** Направление вверх (вдоль положительной оси Y). */
constexpr int BLOCK_DIR_UP = 0x4;
/** Направление вниз (вдоль отрицательной оси Y). */
constexpr int BLOCK_DIR_DOWN = 0x5;

/** Маска для извлечения битов поворота/направления из поля states. */
constexpr int BLOCK_ROTATION_MASK = 0b0000'0111;
/** Маска для зарезервированных битов (могут использоваться для других свойств). */
constexpr int BLOCK_RESERVED_MASK = 0b1111'1000;

/**
 * @brief Структура, представляющая один воксель в мире.
 *
 * Каждый воксель хранит идентификатор типа блока (id) и дополнительные
 * состояния (states), которые могут включать ориентацию, метаданные и т.п.
 */
struct voxel {
    blockid_t id = 0;
    blockstate_t states = 0;

    /**
     * @brief Возвращает текущий поворот/направление блока.
     * @return Значение, хранящееся в младших трёх битах states.
     */
    inline uint8_t rotation() const {
        return states & BLOCK_ROTATION_MASK;
    }

    /**
     * @brief Устанавливает поворот/направление блока.
     * @param rotation Новое значение поворота (должно помещаться в 3 бита).
     */
    inline void setRotation(uint8_t rotation) {
        states = (states & (~BLOCK_ROTATION_MASK)) | (rotation & BLOCK_ROTATION_MASK);
    }
};

#endif // VOXELS_VOXEL_H_
