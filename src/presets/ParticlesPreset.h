#pragma once

#include <glm/vec3.hpp>

#include <interfaces/Serializable.h>

enum ParticleSpawnShape {
    Ball = 0,
    Sphere,
    Box
};

std::string to_string(ParticleSpawnShape shape);
ParticleSpawnShape ParticleSpawnShape_from(std::string_view s);

struct ParticlesPreset : public Serializable {
    bool collision = true;
    bool lighting = true;
    bool globalUpVector = false;
    float maxDistance = 16.0f;
    float spawnInterval = 0.1f;
    float lifetime = 5.0f;
    float lifetimeSpread = 0.2f;
    glm::vec3 velocity {};
    glm::vec3 acceleration {0.0f, -16.0f, 0.0f};
    glm::vec3 explosion {2.0f};
    glm::vec3 size {0.1f};
    ParticleSpawnShape spawnShape;
    glm::vec3 spawnSpread {};
    std::string texture = "";
    float randomSubUV = 1.0f;
    std::vector<std::string> frames {};

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
