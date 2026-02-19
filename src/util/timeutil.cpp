#include "timeutil.h"

#include "../logger/Logger.h"

timeutil::Timer::Timer() {
    start = std::chrono::high_resolution_clock::now();
}
int64_t timeutil::Timer::stop() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
}

timeutil::ScopeLogTimer::ScopeLogTimer(long long id) : scopeid_(id) {
}

timeutil::ScopeLogTimer::~ScopeLogTimer() {
    LOG_DEBUG("Scope {} finished in {} micros.", scopeid_, ScopeLogTimer::stop());
}

float timeutil::time_value(float hour, float minute, float second) {
    return (hour + (minute + second / 60.0f) / 60.0f) / 24.0f;
}

void timeutil::from_value(float value, int& hour, int& minute, int& second) {
    value *= 24;
    hour = value;
    value *= 60;
    minute = int(value) % 60;
    value *= 60;
    second = int(value) % 60;
}
