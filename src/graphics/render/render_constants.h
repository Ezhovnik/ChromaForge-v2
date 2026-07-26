#pragma once

/** @file render_constants.h
 *  @brief Константы графического рендерера.
 *
 *  Определяет параметры сортировки полупрозрачных блоков
 *  и экструзию текстурного атласа.
 */

/** @brief Интервал сортировки полупрозрачных блоков (в кадрах) */
inline constexpr int TRANSLUCENT_BLOCKS_SORT_INTERVAL = 8;
/** @brief Величина экструзии (расширения) тайлов в текстурном атласе */
inline constexpr int ATLAS_EXTRUSION = 2;
