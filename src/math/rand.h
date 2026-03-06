#ifndef MATH_RAND_H_
#define MATH_RAND_H_

#include <random>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "../typedefs.h"

class RandomGenerator {
private:
    static std::mt19937& getGenerator() {
        static std::mt19937 generator(std::random_device{}());
        return generator;
    }
public:
    template<typename T>
    static typename std::enable_if<std::is_integral<T>::value, T>::type
    get(T min, T max) {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(getGenerator());
    }

    template<typename T>
    static typename std::enable_if<std::is_integral<T>::value, T>::type
    get() {
        return get<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    }
};

class FastRandom {
private:
    uint seed;
public:
    inline void setSeed(uint seed) {
        this->seed = seed;
    }

    inline int rand() {
        seed = (214013 * seed + 2531011);
        return (seed >> 16) & 0x7FFF;   
    }

    inline float randFloat() {
        return rand() / float(0x7FFF);
    }
};

#endif // MATH_RAND_H_
