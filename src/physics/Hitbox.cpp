#include <physics/Hitbox.h>

#include <stdexcept>

std::optional<BodyType> BodyType_from(std::string_view str) {
    if (str == "kinematic") {
        return BodyType::Kinematic;
    } else if (str == "dynamic") {
        return BodyType::Dynamic;
    } else if (str == "static") {
        return BodyType::Static;
    }
    return std::nullopt;
}

std::string to_string(BodyType type) {
    switch (type) {
        case BodyType::Kinematic: return "kinematic";
        case BodyType::Dynamic: return "dynamic";
        case BodyType::Static: return "static";
        default: return "unknown";
    }
}

Hitbox::Hitbox(
    BodyType type,
    glm::vec3 position, 
    glm::vec3 halfsize
) : type(type),
    position(position), 
    halfsize(halfsize), 
    velocity(0.0f, 0.0f, 0.0f), 
    linearDamping(0.1f) {}
