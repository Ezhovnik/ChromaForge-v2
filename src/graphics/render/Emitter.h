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

class Emitter {
    const Level& level;
    std::variant<glm::vec3, entityid_t> origin;
    Particle prototype;
    const Texture* texture;
    float spawnInterval;
    int count;
    float timer = 0.0f;
    glm::vec3 explosion {8.0f};
    PseudoRandom random;
public:
    ParticlesPreset preset;

    Emitter(
        const Level& level,
        std::variant<glm::vec3, entityid_t> origin,
        Particle prototype,
        const Texture* texture,
        float spawnInterval,
        int count
    );

    explicit Emitter(const Emitter&) = delete;

    const Texture* getTexture() const;

    void update(
        float delta,
        const glm::vec3& cameraPosition,
        std::vector<Particle>& particles
    ); 

    void setExplosion(const glm::vec3& magnitude);

    bool isDead() const;
};
