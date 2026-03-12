#ifndef SRC_CONSTANTS_H_
#define SRC_CONSTANTS_H_

#include <limits>

#include "typedefs.h"

/**
 * @file constants.h
 * @brief Глобальные константы движка: версия, размеры чанков, идентификаторы и т.д.
 */

// ========== Версия движка ==========

/** Основная версия (major). */
inline constexpr int ENGINE_VERSION_MAJOR = 0;
/** Минорная версия (minor). */
inline constexpr int ENGINE_VERSION_MINOR = 2;
/** Версия поддержки (maintenance). */
inline constexpr int ENGINE_VERSION_MAINTENANCE = 0;
/** Состояние разработки версии. */
const bool ENGINE_VERSION_INDEV = false;

// ========== Размеры чанков ==========

/** Ширина чанка в блоках. */
inline constexpr int CHUNK_WIDTH = 16;
/** Высота чанка в блоках. */
inline constexpr int CHUNK_HEIGHT = 256;
/** Глубина чанка в блоках. */
inline constexpr int CHUNK_DEPTH = 16;
/** Объём чанка (количество блоков). */
inline constexpr int CHUNK_VOLUME = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;

// ========== Биты пользовательских данных в блоке ==========

/** Количество бит, отведённых под пользовательские данные в блоке. */
inline constexpr uint VOXEL_USER_BITS = 8;
/** Смещение пользовательских бит относительно младшего бита. */
inline constexpr uint VOXEL_USER_BITS_OFFSET = sizeof(blockstate_t) * 8 - VOXEL_USER_BITS;

// ========== Специальные идентификаторы блоков и предметов ==========

/** Идентификатор "пустого" блока. */
inline constexpr blockid_t BLOCK_VOID = std::numeric_limits<blockid_t>::max();
/** Идентификатор "пустого" предмета. */
inline constexpr itemid_t ITEM_VOID = std::numeric_limits<itemid_t>::max();
/** Идентификатор воздуха. */
inline constexpr blockid_t BLOCK_AIR = 0;
/** Идентификатор пустого слота предмета. */
inline constexpr itemid_t ITEM_EMPTY = 0;

// ========== Настройки интерфейса ==========

/** Размер иконки предмета в пикселях. */
inline constexpr int ITEM_ICON_SIZE = 48;

// ========== Математические константы ==========

/** Число π. */
inline constexpr double PI = 3.14159265358979323846;

// ========== Утилитарные функции ==========

/**
 * @brief Вычисляет линейный индекс в одномерном массиве по трёхмерным координатам.
 * @param x Координата X (0..w-1).
 * @param y Координата Y (0..h-1).
 * @param z Координата Z (0..d-1).
 * @param w Ширина (по умолчанию CHUNK_WIDTH).
 * @param d Глубина (по умолчанию CHUNK_DEPTH).
 * @return Индекс в плоском массиве.
 *
 * Формула: index = (y * d + z) * w + x.
 */
inline constexpr uint vox_index(uint x, uint y, uint z, uint w = CHUNK_WIDTH, uint d = CHUNK_DEPTH) {
	return (y * d + z) * w + x;
}

// ========== Пути к ресурсам ==========

/** Папка с шейдерами. */
#define SHADERS_FOLDER "shaders"
/** Папка с текстурами. */
#define TEXTURES_FOLDER "textures"
/** Папка со шрифтами. */
#define FONTS_FOLDER "fonts"
/** Папка с макетами интерфейса. */
#define LAYOUTS_FOLDER "layouts"

#endif // SRC_CONSTANTS_H_
