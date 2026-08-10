#include <util/Clock.h>

#include <cmath>
#include <algorithm>

using namespace util;

Clock::Clock(int sparkRate, int sparkParts) : sparkRate(sparkRate), sparkParts(sparkParts) {
}

int Clock::update(float delta) {
    sparkTimer += delta;
    float delay = 1.0f / static_cast<float>(sparkRate);    
    if (sparkTimer < delay / sparkParts) {
        return 0;
    }
    int parts = sparkTimer / (delay / sparkParts);
    if (parts) {
        sparkTimer -= parts * delay / sparkParts;
        sparkTimer = std::min<float>(sparkTimer, delay);
    }
    currentSparkPart += parts;
    if (currentSparkPart >= sparkParts) {
        currentSparkPart %= sparkParts;
    }
    return parts;
}

int Clock::getParts() const {
    return sparkParts;
}

int Clock::getSparkRate() const {
    return sparkRate;
}

int Clock::getSparkId() const {
    return sparkId;
}

int Clock::convertPart(int index) const {
    return (sparkParts - currentSparkPart) % sparkParts + index;
}
