// SimpleProfiler.h
#pragma once
#include <chrono>
#include <iostream>
#include <string>

struct ScopedPerfSample {
    std::chrono::high_resolution_clock::time_point start;
    std::string name;
    ScopedPerfSample(const std::string& n) : name(n), start(std::chrono::high_resolution_clock::now()) {}
    ~ScopedPerfSample() {
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        // Выводим только если заняло больше 10 мс (чтобы не спамить мелочью)
        if (ms > 10.0) {
            std::cout << "[PERF] " << name << " : " << ms << " ms" << std::endl;
        }
    }
};

#define PROFILE_SCOPE(name) ScopedPerfSample __perf_sample_##__LINE__(name)
