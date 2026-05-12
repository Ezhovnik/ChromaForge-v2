#pragma once

#include <vector>
#include <variant>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <math/UVRegion.h>
#include <math/rand.h>
#include <presets/ParticlesPreset.h>

class Emitter;
class Level;

struct Particle {
    Emitter* emitter;
    int random;
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
    UVRegion region;
};

class Texture;

using EmitterOrigin = std::variant<glm::vec3, entityid_t>;

class Emitter {
    const Level& level;
    EmitterOrigin origin;
    Particle prototype;
    const Texture* texture;
    int count;
    float timer = 0.0f;
    PseudoRandom random;
public:
    ParticlesPreset preset;

    Emitter(
        const Level& level,
        std::variant<glm::vec3, entityid_t> origin,
        ParticlesPreset preset,
        const Texture* texture,
        const UVRegion& region,
        int count
    );

    explicit Emitter(const Emitter&) = delete;

    const Texture* getTexture() const;

    void update(
        float delta,
        const glm::vec3& cameraPosition,
        std::vector<Particle>& particles
    );

    void stop();

    bool isDead() const;
};
