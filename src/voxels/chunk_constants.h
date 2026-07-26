#pragma once

/** @file chunk_constants.h
 *  @brief Константы размеров чанков и карты чанков.
 *
 *  Определяет геометрию одного чанка (ширина, высота, глубина, объём)
 *  и коэффициент максимальной загрузки хеш-карты чанков.
 */

/** @brief Ширина чанка в блоках */
inline constexpr int CHUNK_WIDTH = 16;
/** @brief Высота чанка в блоках */
inline constexpr int CHUNK_HEIGHT = 256;
/** @brief Глубина чанка в блоках */
inline constexpr int CHUNK_DEPTH = 16;
/** @brief Объём чанка (ширина * высота * глубина) */
inline constexpr int CHUNK_VOLUME = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;

/** @brief Максимальный коэффициент загрузки для std::unordered_map чанков */
inline constexpr float CHUNKS_MAP_MAX_LOAD_FACTOR = 0.1f;
