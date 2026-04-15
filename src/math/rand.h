#ifndef MATH_RAND_H_
#define MATH_RAND_H_

#include <random>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <ctime>

#include "typedefs.h"

/**
 * @brief Генератор случайных чисел на основе std::mt19937 (вихрь Мерсенна).
 *
 * Предоставляет удобные статические методы для получения случайных целых чисел
 * в заданном диапазоне или во всём диапазоне типа.
 *
 * @note Все методы используют один статический генератор, что может вызывать
 *       проблемы при одновременном доступе из нескольких потоков.
 */
class RandomGenerator {
private:
    /**
     * @brief Возвращает ссылку на статический генератор.
     * @return Ссылка на std::mt19937.
     */
    static std::mt19937& getGenerator() {
        static std::mt19937 generator(std::random_device{}());
        return generator;
    }
public:
    /**
     * @brief Возвращает случайное целое число в заданном диапазоне [min, max].
     * @tparam T Целочисленный тип.
     * @param min Минимальное значение (включительно).
     * @param max Максимальное значение (включительно).
     * @return Случайное число типа T.
     */
    template<typename T>
    static typename std::enable_if<std::is_integral<T>::value, T>::type
    get(T min, T max) {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(getGenerator());
    }

    /**
     * @brief Возвращает случайное целое число во всём диапазоне типа T.
     * @tparam T Целочисленный тип.
     * @return Случайное число от std::numeric_limits<T>::min() до max().
     */
    template<typename T>
    static typename std::enable_if<std::is_integral<T>::value, T>::type
    get() {
        return get<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    }
};

/**
 * @brief Быстрый линейный конгруэнтный генератор (LCG).
 * 
 * Период: 2^32.
 */
class FastRandom {
private:
    uint seed; ///< Текущее состояние генератора.
public:
    /**
     * @brief Устанавливает начальное значение (seed).
     * @param seed Новое значение seed.
     */
    inline void setSeed(uint seed) {
        this->seed = seed;
    }

    /**
     * @brief Генерирует следующее случайное целое число в диапазоне [0, 0x7FFF].
     * @return Случайное число от 0 до 32767.
     */
    inline int rand() {
        seed = (214013 * seed + 2531011);
        return (seed >> 16) & 0x7FFF;   
    }

    /**
     * @brief Генерирует случайное число с плавающей точкой в диапазоне [0, 1].
     * @return Значение от 0.0 до 1.0.
     */
    inline float randFloat() {
        return rand() / float(0x7FFF);
    }
};

/**
 * @brief Псевдослучайный генератор с собственным алгоритмом перемешивания.
 *
 * Использует 16-битное состояние и серию арифметических операций для генерации.
 * Предоставляет методы для получения 32- и 64-битных значений.
 */
class PseudoRandom {
private:
	ushort seed; ///< 16-битное состояние.
public:
    /**
     * @brief Конструктор, инициализирующий seed текущим временем.
     */
	PseudoRandom() {seed = (ushort)time(0);}

    /**
     * @brief Генерирует следующее 16-битное псевдослучайное число.
     * @return Значение в диапазоне 0..65535.
     */
	int rand(){
		seed = (seed + 0x7ed5 + (seed << 6));
		seed = (seed ^ 0xc23c ^ (seed >> 9));
		seed = (seed + 0x1656 + (seed << 3));
		seed = ((seed + 0xa264) ^ (seed << 4));
		seed = (seed + 0xfd70 - (seed << 3));
		seed = (seed ^ 0xba49 ^ (seed >> 8));

		return (int)seed;
	}

    /**
     * @brief Генерирует 32-битное знаковое целое.
     * @return Случайное число в диапазоне int32.
     */
    int32_t rand32() {
        return (rand() << 16) | rand();
    }

    /**
     * @brief Генерирует 32-битное беззнаковое целое.
     * @return Случайное число в диапазоне uint32.
     */
    uint32_t randU32() {
        return (rand() << 16) | rand();
    }

    /**
     * @brief Генерирует 64-битное знаковое целое.
     * @return Случайное число в диапазоне int64.
     */
    int64_t rand64() {
        uint64_t x = randU32();
        uint64_t y = randU32();
        return (x << 32ULL) | y;
    }

    /**
     * @brief Устанавливает seed по одному числу.
     * @param number Исходное значение для инициализации.
     */
	void setSeed(int number){
		seed = ((ushort)(number * 3729) ^ (ushort)(number + 16786));
		rand(); // Прогон для улучшения распределения
	}

    /**
     * @brief Устанавливает seed по двум числам.
     * @param number1 Первое число.
     * @param number2 Второе число.
     */
	void setSeed(int number1, int number2){
		seed = (((ushort)(number1 * 23729) | (ushort)(number2 % 16786)) ^ (ushort)(number2 * number1));
		rand(); // Прогон для улучшения распределения
	}
};

#endif // MATH_RAND_H_
