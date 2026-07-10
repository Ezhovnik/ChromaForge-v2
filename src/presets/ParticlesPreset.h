#pragma once

#include <string>
#include <vector>

#include <glm/vec3.hpp>

#include <interfaces/Serializable.h>
#include <util/EnumMetadata.h>

enum class ParticleSpawnShape {
    Ball = 0,
    Sphere,
    Box
};

CHROMA_ENUM_METADATA(ParticleSpawnShape)
    {"ball", ParticleSpawnShape::Ball},
    {"sphere", ParticleSpawnShape::Sphere},
    {"box", ParticleSpawnShape::Box},
CHROMA_ENUM_END

struct ParticlesPreset : public Serializable {
    bool collision = true;
    bool lighting = true;
    bool globalUpVector = false;
    float maxDistance = 32.0f;
    float spawnInterval = 0.1f;
    float lifetime = 5.0f;
    float lifetimeSpread = 0.2f;
    glm::vec3 velocity {};
    glm::vec3 acceleration {0.0f, -16.0f, 0.0f};
    glm::vec3 explosion {2.0f};
    glm::vec3 size {0.1f};
    float sizeSpread = 0.2f;
    float angleSpread = 0.0f;
    float minAngularVelocity = 0.0f;
    float maxAngularVelocity = 0.0f;
    ParticleSpawnShape spawnShape = ParticleSpawnShape::Ball;
    glm::vec3 spawnSpread {};
    std::string texture = "";
    float randomSubUV = 1.0f;
    std::vector<std::string> frames;

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
