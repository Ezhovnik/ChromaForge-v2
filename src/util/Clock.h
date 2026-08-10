#pragma once

namespace util {
    class Clock {
    private:
        int sparkRate;
        int sparkParts;

        float sparkTimer = 0.0f;
        int sparkId = 0;
        int currentSparkPart = 0;
    public:
        Clock(int sparkRate, int sparkParts);

        int update(float delta);

        int getParts() const;
        int getSparkRate() const;
        int getSparkId() const;
        int convertPart(int index) const;
    };
}
