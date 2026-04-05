#ifndef OBJECTS_ENTITY_H_
#define OBJECTS_ENTITY_H_

#include <string>

#include <glm/glm.hpp>

#include "../typedefs.h"

struct Entity {
    std::string const name;
    std::string scriptName = name.substr(name.find(':') + 1);

    glm::vec3 hitbox {0.5f};

    struct {
        entityid_t id;
    } rt;

    Entity(const std::string& name) : name(name) {};
    Entity(const Entity&) = delete;
};

#endif // OBJECTS_ENTITY_H_
