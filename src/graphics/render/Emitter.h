#pragma once

#include <vector>
#include <variant>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <math/UVRegion.h>

class Emitter;

struct Particle {
    Emitter* emitter;
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
    UVRegion region;
};

class Texture;

class Emitter {
    std::variant<glm::vec3, entityid_t> origin;
    Particle prototype;
    const Texture* texture;
    float spawnInterval;
    int count;
    float timer = 0.0f;
    glm::vec3 explosion {1.0f};
public:
    Emitter(
        std::variant<glm::vec3, entityid_t> origin,
        Particle prototype,
        const Texture* texture,
        float spawnInterval,
        int count
    );

    const Texture* getTexture() const;

    void update(float delta, std::vector<Particle>& particles);    

    void setExplosion(const glm::vec3& magnitude);

    bool isDead() const;
};
