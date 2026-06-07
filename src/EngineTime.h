#pragma once

#include <stdint.h>

class EngineTime {
    uint64_t frame = 0;
    double lastTime = 0.0;
    double deltaTime = 0.0;
public:
    EngineTime() {}

    void update(double currentTime) {
        ++frame;
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
    }

    void step(double delta) {
        ++frame;
        lastTime += delta;
    }

    void set(double currentTime) {
        lastTime = currentTime;
    }

    double getDeltaTime() const {
        return deltaTime;
    }

    double getTime() const {
        return lastTime;
    }
};
