#pragma once

/**
 * @file voxmaths.h
 * @brief Простые математические утилиты для работы с воксельными данными.
 *
 * Содержит функции целочисленного деления с округлением вниз/вверх.
 */

/**
 * @brief Целочисленное деление с округлением вниз (к отрицательной бесконечности).
 * @param a Делимое.
 * @param b Делитель (должен быть положительным).
 * @return Наибольшее целое q, такое что q * b <= a.
 *
 * Примеры:
 * floordiv(7, 3) = 2
 * floordiv(-7, 3) = -3
 */
inline int floordiv(int a, int b) {
	if (a < 0 && a % b) return (a / b) - 1;
	return a / b;
}

inline constexpr bool is_pot(int a) {
    return (a > 0) && ((a & (a - 1)) == 0);
}

inline constexpr unsigned floorlog2(unsigned x) {
    return x == 1 ? 0 : 1 + floorlog2(x >> 1);
}

template<int b>
inline constexpr int floordiv(int a) {
    if constexpr (is_pot(b)) {
        return a >> floorlog2(b);
    } else {
        return floordiv(a, b);
    }
}

/**
 * @brief Целочисленное деление с округлением вверх (к положительной бесконечности).
 * @param a Делимое.
 * @param b Делитель (должен быть положительным).
 * @return Наименьшее целое q, такое что q * b >= a.
 *
 * Примеры:
 * ceildiv(7, 3) = 3
 * ceildiv(-7, 3) = -2
 */
inline int ceildiv(int a, int b) {
	if (a > 0 && a % b) return a / b + 1;
	return a / b;
}
