#include "Clock.h"

#include <cmath>

using namespace util;

Clock::Clock(int sparkRate, int sparkParts) : sparkRate(sparkRate), sparkParts(sparkParts) {
}

bool Clock::update(float delta) {
    sparkTimer += delta;
    float delay = 1.0f / float(sparkRate);    
    if (sparkTimer > delay || sparkPartsUndone) {
        if (sparkPartsUndone) {
            sparkPartsUndone--;
        } else {
            sparkTimer = std::fmod(sparkTimer, delay);
            sparkPartsUndone = sparkParts - 1;
        }
        return true;
    }
    return false;
}

int Clock::getParts() const {
    return sparkParts;
}

int Clock::getPart() const {
    return sparkParts-sparkPartsUndone-1;
}

int Clock::getSparkRate() const {
    return sparkRate;
}

int Clock::getSparkId() const {
    return sparkId;
}
