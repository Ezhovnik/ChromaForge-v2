#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace util {
    constexpr inline float EPSILON = 1e-6f;

    inline int distance2(const glm::ivec3& a, const glm::ivec3& b) {
        return (b.x - a.x) * (b.x - a.x) +
                (b.y - a.y) * (b.y - a.y) +
                (b.z - a.z) * (b.z - a.z);
    }

    inline int distance2(int ax, int ay, int az, int bx, int by, int bz) {
        return (bx - ax) * (bx - ax) +
               (by - ay) * (by - ay) +
               (bz - az) * (bz - az);
    }

    inline int length2(const glm::ivec3& a) {
        return a.x * a.x + a.y * a.y + a.z * a.z;
    }

    inline int length2(int x, int y, int z) {
        return x * x + y * y + z * z;
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
