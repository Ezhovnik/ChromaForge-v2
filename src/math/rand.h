#ifndef MATH_RAND_H_
#define MATH_RAND_H_

#include <random>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <ctime>

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

class PseudoRandom {
private:
	ushort seed;
public:
	PseudoRandom() {seed = (ushort)time(0);}

	int rand(){
		seed = (seed + 0x7ed5 + (seed << 6));
		seed = (seed ^ 0xc23c ^ (seed >> 9));
		seed = (seed + 0x1656 + (seed << 3));
		seed = ((seed + 0xa264) ^ (seed << 4));
		seed = (seed + 0xfd70 - (seed << 3));
		seed = (seed ^ 0xba49 ^ (seed >> 8));

		return (int)seed;
	}

    int32_t rand32() {
        return (rand() << 16) | rand();
    }

    uint32_t randU32() {
        return (rand() << 16) | rand();
    }

    int64_t rand64() {
        uint64_t x = randU32();
        uint64_t y = randU32();
        return (x << 32ULL) | y;
    }

	void setSeed(int number){
		seed = ((ushort)(number * 3729) ^ (ushort)(number + 16786));
		rand();
	}
	void setSeed(int number1, int number2){
		seed = (((ushort)(number1 * 23729) | (ushort)(number2 % 16786)) ^ (ushort)(number2 * number1));
		rand();
	}
};

#endif // MATH_RAND_H_
