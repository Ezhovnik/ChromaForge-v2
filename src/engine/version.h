#pragma once

/** @file version.h
 *  @brief Константы версии движка.
 *
 *  Содержит мажорную, минорную и патч-версию, строковое представление
 *  и флаг отладочной сборки.
 */

#include <string>

/** @brief Мажорная версия движка */
inline constexpr int ENGINE_VERSION_MAJOR = 0;
/** @brief Минорная версия движка */
inline constexpr int ENGINE_VERSION_MINOR = 4;
/** @brief Патч-версия движка */
inline constexpr int ENGINE_VERSION_PATCH = 0;
/** @brief Строковое представление версии */
inline const std::string ENGINE_VERSION_STRING = std::to_string(ENGINE_VERSION_MAJOR) + "." + std::to_string(ENGINE_VERSION_MINOR) + "." + std::to_string(ENGINE_VERSION_PATCH);

/** @brief Флаг отладочной сборки.
 *  true — если сборка отладочная (NDEBUG не определён),
 *  false — если релизная.
 */
#ifdef NDEBUG
inline constexpr bool ENGINE_DEBUG_BUILD = false;
#else
inline constexpr bool ENGINE_DEBUG_BUILD = true;
#endif
