#pragma once

#include <random>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <array>
#include <algorithm>
#include <ctime>

#include <typedefs.h>

namespace util {
    inline uint64_t shuffle_bits_step(uint64_t x, uint64_t m, unsigned shift) {
        uint64_t t = ((x >> shift) ^ x) & m;
        x = (x ^ t) ^ (t << shift);
        return x;
    }

    /**
     * @brief Генератор случайных чисел на основе std::mt19937 (вихрь Мерсенна).
     */
    template<typename Engine = std::mt19937>
    class RandomGenerator {
    public:
        /**
         * @brief Создаёт генератор с seed'ом из std::random_device + seed_seq.
         */
        RandomGenerator() : m_engine(make_seeded_engine()) {}

        /**
         * @brief Создаёт генератор с явным seed'ом.
         * @param seed Значение для инициализации.
         */
        explicit RandomGenerator(typename Engine::result_type seed)
            : m_engine(seed) {}

        /**
         * @brief Возвращает ссылку на внутренний движок.
         */
        Engine& engine() {
            return m_engine;
        }

        /**
         * @brief Возвращает случайное целое число в диапазоне [min, max].
         */
        template<typename T>
        std::enable_if_t<std::is_integral_v<T>, T>
        next(T min, T max) {
            using WideT = std::conditional_t<sizeof(T) < sizeof(int), int, T>;
            std::uniform_int_distribution<WideT> dist(static_cast<WideT>(min), static_cast<WideT>(max));
            return static_cast<T>(dist(m_engine));
        }

        /**
         * @brief Возвращает случайное целое число во всём диапазоне типа T.
         */
        template<typename T>
        std::enable_if_t<std::is_integral_v<T>, T>
        next() {
            return next<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
        }

    private:
        Engine m_engine;

        static Engine make_seeded_engine() {
            std::random_device source;
            using rd_result = std::random_device::result_type;
            constexpr std::size_t seed_bytes = Engine::state_size * sizeof(typename Engine::result_type);
            constexpr std::size_t num_seeds = (seed_bytes - 1) / sizeof(rd_result) + 1;
            rd_result data[num_seeds];
            std::generate(std::begin(data), std::end(data), std::ref(source));
            std::seed_seq seq(std::begin(data), std::end(data));
            return Engine(seq);
        }
    };

    std::string generate_uuid();

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
        PseudoRandom(unsigned short seed) : seed(seed) {}

        /**
         * @brief Конструктор, инициализирующий seed текущим временем.
         */
        PseudoRandom() {seed = static_cast<unsigned short>(time(nullptr));}

        /**
         * @brief Генерирует следующее 16-битное псевдослучайное число.
         * @return Значение в диапазоне 0..65535.
         */
        int rand() {
            seed = (seed + 0x7ed5 + (seed << 6));
            seed = (seed ^ 0xc23c ^ (seed >> 9));
            seed = (seed + 0x1656 + (seed << 3));
            seed = ((seed + 0xa264) ^ (seed << 4));
            seed = (seed + 0xfd70 - (seed << 3));
            seed = (seed ^ 0xba49 ^ (seed >> 8));

            return static_cast<int>(seed);
        }

        void rand(unsigned char* dst, size_t n) {
            for (size_t i = 0; i < n; ++i) {
                dst[i] = rand();
            }
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

        uint64_t randU64() {
            uint64_t x = randU32();
            uint64_t y = randU32();
            return (x << 32ULL) | y;
        }

        float randFloat() {
            return randU32() / static_cast<float>(UINT32_MAX);
        }

        double randDouble() {
            return randU64() / static_cast<double>(UINT64_MAX);
        }

        /**
         * @brief Устанавливает seed по двум числам.
         * @param number1 Первое число.
         * @param number2 Второе число.
         */
        void setSeed(int number1, int number2){
            seed = ((static_cast<ushort>(number1 * 23729) | static_cast<ushort>(number2 % 16786)) ^ static_cast<ushort>(number2 * number1));
            rand(); // Прогон для улучшения распределения
        }

        /**
         * @brief Устанавливает seed по одному числу.
         * @param number Исходное значение для инициализации.
         */
        void setSeed(long number) {
            number = util::shuffle_bits_step(number, 0x2222222222222222ull, 1);
            number = util::shuffle_bits_step(number, 0x0c0c0c0c0c0c0c0cull, 2);
            number = util::shuffle_bits_step(number, 0x00f000f000f000f0ull, 4);
            seed = number;
            rand();
        }
    };
} // namespace util
