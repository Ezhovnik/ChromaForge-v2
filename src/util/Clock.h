#pragma once

namespace util {
    class Clock {
    private:
        int sparkRate;
        int sparkParts;

        float sparkTimer = 0.0f;
        int sparkId = 0;
        int sparkPartsUndone = 0;
    public:
        Clock(int sparkRate, int sparkParts);

        bool update(float delta);

        int getParts() const;
        int getPart() const;
        int getSparkRate() const;
        int getSparkId() const;
    };
}
