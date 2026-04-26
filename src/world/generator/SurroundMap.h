#pragma once

#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

class SurroundMap {
    struct Entry {
        int level = 0;
        uint8_t surrounding = 0x0;
        bool confirmed = false;
        bool marked = false;
    };
    std::unordered_map<glm::ivec2, Entry> entries;
public:
    void resetMarks();

    void markIsle(const glm::ivec2& origin);

    std::vector<glm::ivec2> sweep();

    std::vector<glm::ivec2> upgrade();

    bool createEntry(const glm::ivec2& origin);

    void getLevels(unsigned char* out, int width, int height, int ox, int oy) const;
};
