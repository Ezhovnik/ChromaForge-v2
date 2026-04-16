#ifndef UTIL_CLOCK_H_
#define UTIL_CLOCK_H_

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

#endif // UTIL_CLOCK_H_
