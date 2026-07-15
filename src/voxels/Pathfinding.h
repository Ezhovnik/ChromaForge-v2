#pragma once

#include <vector>
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class Level;

namespace voxels {
    struct RouteNode {
        glm::ivec3 pos;
    };

    struct Route {
        bool found;
        std::vector<RouteNode> nodes;
    };

    struct Map {
        int width;
        int height;
        std::unique_ptr<uint8_t[]> map;

        Map(
            int width, int height
        ) : width(width),
            height(height),
            map(std::make_unique<uint8_t[]>(width * height)) {}

        uint8_t& operator[](int i) {
            return map[i];
        }

        const uint8_t& operator[](int i) const {
            return map[i];
        }
    };

    class Pathfinding {
    public:
        Pathfinding(const Level& level);

        Route perform(const glm::ivec3& start, const glm::ivec3& end);
    private:
        const Level& level;
    };
}
