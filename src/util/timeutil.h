#ifndef UTIL_TIMEUTIL_H_
#define UTIL_TIMEUTIL_H_

#include <chrono>

namespace timeutil {
    class Timer {
    private:
        std::chrono::high_resolution_clock::time_point start;
    public:
        Timer();
        int64_t stop();
    };

    class ScopeLogTimer : public Timer{
    private:
        long long scopeid_;
    public:
        ScopeLogTimer(long long id);
        ~ScopeLogTimer();
    };

    float time_value(float hour, float minute, float second);
    void from_value(float value, int& hour, int& minute, int& second);
}

#endif // UTIL_TIMEUTIL_H_
