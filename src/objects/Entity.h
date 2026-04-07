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
    std::string scriptName = name.substr(name.find(':') + 1);
    std::string rigName = name.substr(name.find(":") + 1);

    glm::vec3 hitbox {0.5f};
    std::vector<AABB> triggers {};

    struct {
        entityid_t id;
        rigging::RigConfig* rig;
    } rt {};

    Entity(const std::string& name) : name(name) {};
    Entity(const Entity&) = delete;
};

#endif // OBJECTS_ENTITY_H_
