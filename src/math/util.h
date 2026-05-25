#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace util {
    template<typename T>
    inline T sqr(T value) {
        return value * value;
    }

    constexpr inline float EPSILON = 1e-6f;

    inline int distance2(const glm::ivec3& a, const glm::ivec3& b) {
        return sqr(b.x - a.x) + sqr(b.y - a.y) + sqr(b.z - a.z);
    }

    inline int distance2(int ax, int ay, int az, int bx, int by, int bz) {
        return sqr(bx - ax) + sqr(by - ay) + sqr(bz - az);
    }

    inline int length2(const glm::ivec3& a) {
        return sqr(a.x) + sqr(a.y) + sqr(a.z);
    }

    inline int length2(int x, int y, int z) {
        return sqr(x) + sqr(y) + sqr(z);
    }

    inline glm::vec3 closest_point_on_segment(
        const glm::vec3& a, const glm::vec3& b, const glm::vec3& point
    ) {
        auto vec = b - a;
        float da = glm::distance2(point, a);
        float db = glm::distance2(point, b);
        float len = glm::length2(vec);
        float t = (((da - db) / len) * 0.5f + 0.5f);
        t = std::min(1.0f, std::max(0.0f, t));
        return a + vec * t;
    }

    inline glm::ivec3 closest_point_on_segment(
        const glm::ivec3& a, const glm::ivec3& b, const glm::ivec3& point
    ) {
        auto vec = b - a;
        float da = distance2(point, a);
        float db = distance2(point, b);
        float len = length2(vec);
        float t = (((da - db) / len) * 0.5f + 0.5f);
        t = std::min(1.0f, std::max(0.0f, t));
        return a + glm::ivec3(glm::vec3(vec) * t);
    }
};
