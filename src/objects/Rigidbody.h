#pragma once

#include <vector>

#include <entt/fwd.hpp>

#include <data/dv_fwd.h>
#include <physics/Hitbox.h>

class Entities;
struct Entity;

struct Rigidbody {
    bool enabled = true;
    Hitbox hitbox;
    std::vector<Sensor> sensors;

    dv::value serialize(bool saveVelocity, bool saveBodySettings) const;
    void deserialize(const dv::value& root);

    void initialize(
        const Entity& def,
        entityid_t id,
        Entities& entities
    );
};
