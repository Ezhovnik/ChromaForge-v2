#ifndef OBJECTS_ENTITY_H_
#define OBJECTS_ENTITY_H_

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "../typedefs.h"
#include "../math/AABB.h"

namespace rigging {
    class RigConfig;
}

struct Entity {
    std::string const name;
    std::vector<std::string> components;
    std::string rigName = name;

    glm::vec3 hitbox {0.5f};
    std::vector<std::pair<size_t, AABB>> boxTriggers {};
    std::vector<std::pair<size_t, float>> radialTriggers {};

    struct {
        bool enabled = true;
        struct {
            bool textures = false;
            bool pose = false;
        } rig;
    } save {};

    struct {
        entityid_t id;
        rigging::RigConfig* rig;
    } rt {};

    Entity(const std::string& name) : name(name) {};
    Entity(const Entity&) = delete;
};

#endif // OBJECTS_ENTITY_H_
