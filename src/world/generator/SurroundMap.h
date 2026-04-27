#pragma once

#include <unordered_map>
#include <functional>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <typedefs.h>
#include <util/AreaMap2D.h>

class SurroundMap {
public:
    using LevelCallback = std::function<void(int, int)>;
    struct LevelCallbackWrapper {
        LevelCallback callback;
        bool active = false;
    };
private:
    util::AreaMap2D<int8_t> areaMap;
    std::vector<LevelCallbackWrapper> levelCallbacks;
    int8_t maxLevel;

    void upgrade(int x, int y, int8_t level);
public:
    SurroundMap(int maxLevelRadius, int8_t maxLevel);

    void setLevelCallback(int8_t level, LevelCallback callback);
    void setOutCallback(util::AreaMap2D<int8_t>::OutCallback callback);

    void completeAt(int x, int y);

    void setCenter(int x, int y);

    void resize(int maxLevelRadius);

    int8_t at(int x, int y);

    const util::AreaMap2D<int8_t>& getArea() const {
        return areaMap;
    }
};
