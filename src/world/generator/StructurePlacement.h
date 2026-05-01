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
