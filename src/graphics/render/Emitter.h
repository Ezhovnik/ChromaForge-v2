#pragma once

#include <vector>
#include <variant>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <math/UVRegion.h>
#include <math/rand.h>

class Emitter;

struct Particle {
    Emitter* emitter;
    int random;
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
    UVRegion region;
};

class Texture;

struct ParticleBehaviour {
    bool collision = true;
    bool lighting = true;
    glm::vec3 gravity {0.0f, -16.0f, 0.0f};
};

class Emitter {
    std::variant<glm::vec3, entityid_t> origin;
    Particle prototype;
    const Texture* texture;
    float spawnInterval;
    int count;
    float timer = 0.0f;
    glm::vec3 explosion {8.0f};
    PseudoRandom random;
public:
    ParticleBehaviour behaviour;

    Emitter(
        std::variant<glm::vec3, entityid_t> origin,
        Particle prototype,
        const Texture* texture,
        float spawnInterval,
        int count
    );

    explicit Emitter(const Emitter&) = delete;

    const Texture* getTexture() const;

    void update(float delta, std::vector<Particle>& particles);    

    void setExplosion(const glm::vec3& magnitude);

    bool isDead() const;
};
