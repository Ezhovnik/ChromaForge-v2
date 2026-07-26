#pragma once

/** @file content_constants.h
 *  @brief Константы-идентификаторы для блоков, предметов и сущностей.
 *
 *  Определяет sentinel-значения (пустые/недействительные ID),
 *  а также ID стандартных блоков (воздух, препятствие и т.д.).
 */

#include <limits>
#include <typedefs.h>

/** @brief Маркер "нет блока" (максимальное значение blockid_t) */
inline constexpr blockid_t BLOCK_VOID = std::numeric_limits<blockid_t>::max();
/** @brief Маркер "нет предмета" (максимальное значение itemid_t) */
inline constexpr itemid_t ITEM_VOID = std::numeric_limits<itemid_t>::max();
/** @brief ID блока воздуха */
inline constexpr blockid_t BLOCK_AIR = 0;
/** @brief ID блока-препятствия */
inline constexpr blockid_t BLOCK_OBSTACLE = 1;
/** @brief ID структурного воздуха (используется в структурах) */
inline constexpr blockid_t BLOCK_STRUCT_AIR = 2;
/** @brief ID пустого слота предмета */
inline constexpr itemid_t ITEM_EMPTY = 0;
/** @brief Маркер "нет сущности" */
inline constexpr entityid_t ENTITY_NONE = 0;
/** @brief Маркер "автоматический ID сущности" (максимальное значение entityid_t) */
inline constexpr entityid_t ENTITY_AUTO = std::numeric_limits<entityid_t>::max();
