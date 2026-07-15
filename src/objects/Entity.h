#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <math/AABB.h>
#include <physics/Hitbox.h>
#include <data/dv.h>

namespace rigging {
    class SkeletonConfig;
}

struct ComponentInstance {
    std::string component;
    dv::value params;
};

struct Entity {
    std::string const name;
    std::vector<ComponentInstance> components;
    std::string skeletonName = name;
    bool blocking = true;

    BodyType bodyType = BodyType::Dynamic;
    glm::vec3 hitbox {0.25f};
    std::vector<std::pair<size_t, AABB>> boxSensors {};
    std::vector<std::pair<size_t, float>> radialSensors {};

    struct {
        bool enabled = true;
        struct {
            bool textures = false;
            bool pose = false;
        } skeleton;
        struct {
            bool velocity = true;
            bool settings = true;
        } body;
    } save {};

    struct {
        entityid_t id;
        rigging::SkeletonConfig* skeleton;
    } rt {};

    Entity(const std::string& name) : name(name) {};
    Entity(const Entity&) = delete;

    void cloneTo(Entity& dst);
};
