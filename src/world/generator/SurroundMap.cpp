#include <world/generator/SurroundMap.h>

#include <queue>
#include <cstring>
#include <stdexcept>

#include <debug/Logger.h>

SurroundMap::SurroundMap(
    int maxLevelRadius,
    int8_t maxLevel
) : areaMap((maxLevelRadius + maxLevel) * 2 + 1, (maxLevelRadius + maxLevel) * 2 + 1),
    levelCallbacks(maxLevel),
    maxLevel(maxLevel) 
{}

void SurroundMap::setLevelCallback(int8_t level, LevelCallback callback) {
    auto& wrapper = levelCallbacks.at(level - 1);
    wrapper.callback = callback;
    wrapper.active = callback != nullptr;
}

void SurroundMap::setOutCallback(util::AreaMap2D<int8_t>::OutCallback callback) {
    areaMap.setOutCallback(callback);
}

void SurroundMap::upgrade(int x, int y, int8_t level) {
    auto& callback = levelCallbacks[level - 1];
    int size = maxLevel - level + 1;
    for (int ly = -size + 1; ly < size; ++ly) {
        for (int lx = -size + 1; lx < size; ++lx) {
            int posX = lx + x;
            int posY = ly + y;
            int8_t sourceLevel = areaMap.get(posX, posY, 0);
            if (sourceLevel < level - 1) {
                LOG_ERROR("Invalid map state");
                throw std::runtime_error("Invalid map state");
            }
            if (sourceLevel >= level) continue;
            areaMap.set(posX, posY, level);
            if (callback.active) {
                callback.callback(posX, posY);
            }
        }
    }
}

void SurroundMap::resize(int maxLevelRadius) {
    areaMap.resize((maxLevelRadius + maxLevel) * 2 + 1, (maxLevelRadius + maxLevel) * 2 + 1);
}

void SurroundMap::completeAt(int x, int y) {
    if (!areaMap.isInside(x - maxLevel + 1, y - maxLevel + 1) || !areaMap.isInside(x + maxLevel - 1, y + maxLevel - 1)) {
        LOG_ERROR("Upgrade square is not fully inside of area");
        throw std::invalid_argument("Upgrade square is not fully inside of area");
    }
    for (int8_t level = 1; level <= maxLevel; ++level) {
        upgrade(x, y, level);
    }
}

void SurroundMap::setCenter(int x, int y) {
    areaMap.setCenter(x, y);
}

int8_t SurroundMap::at(int x, int y) {
    if (auto ptr = areaMap.getIf(x, y)) {
        return *ptr;
    }
    LOG_ERROR("Position is out of area");
    throw std::invalid_argument("Position is out of area");
}
