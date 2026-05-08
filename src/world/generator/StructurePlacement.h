#pragma once

#include <glm/glm.hpp>
#include <stdint.h>

struct StructurePlacement {
    int structure;

    glm::ivec3 position;
    uint8_t rotation;

    StructurePlacement(
        int structure,
        glm::ivec3 position,
        uint8_t rotation
    ) : structure(structure),
        position(std::move(position)),
        rotation(rotation) {}
};

struct LinePlacement {
    blockid_t block;
    glm::ivec3 a;
    glm::ivec3 b;
    int radius;

    LinePlacement(blockid_t block, glm::ivec3 a, glm::ivec3 b, int radius) : block(block), a(std::move(a)), b(std::move(b)), radius(radius) {
    }
};
