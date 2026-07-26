#pragma once

/** @file voxel_constants.h
 *  @brief Константы для работы с данными вокселей.
 *
 *  Определяет количество пользовательских бит в блоке состояния,
 *  их смещение и вспомогательную функцию индексации вокселей.
 */

#include <typedefs.h>
#include <voxels/chunk_constants.h>

/** @brief Количество бит, отведённых под пользовательские данные в blockstate_t */
inline constexpr uint VOXEL_USER_BITS = 8;
/** @brief Смещение пользовательских бит от младшего бита blockstate_t */
inline constexpr uint VOXEL_USER_BITS_OFFSET = sizeof(blockstate_t) * 8 - VOXEL_USER_BITS;

/** @brief Преобразует трёхмерные координаты в линейный индекс массива.
 *  @param x Координата X
 *  @param y Координата Y
 *  @param z Координата Z
 *  @param w Ширина (по умолчанию CHUNK_WIDTH)
 *  @param d Глубина (по умолчанию CHUNK_DEPTH)
 *  @return Линейный индекс в массиве объёмом w * d * ... */
inline constexpr uint vox_index(uint x, uint y, uint z, uint w = CHUNK_WIDTH, uint d = CHUNK_DEPTH) {
	return (y * d + z) * w + x;
}
