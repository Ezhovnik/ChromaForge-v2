#pragma once

#include <glm/vec3.hpp>

#include <interfaces/Serializable.h>

struct ParticlesPreset : public Serializable {
    bool collision = true;
    bool lighting = true;
    float maxDistance = 32.0f;
    glm::vec3 acceleration {0.0f, -16.0f, 0.0f};

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
