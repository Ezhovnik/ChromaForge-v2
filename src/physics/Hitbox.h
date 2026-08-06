#pragma once

#include <set>
#include <functional>
#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include <math/AABB.h>
#include <typedefs.h>
#include <util/EnumMetadata.h>

enum class SensorType {
    AABB,
    RADIUS,
};

union SensorParams {
    AABB aabb;
    glm::vec4 radial;

    constexpr SensorParams() : aabb() {
    }
};

using sensorcallback = std::function<void(entityid_t, size_t, entityid_t)>;

struct Sensor {
    bool enabled = true;
    SensorType type;
    size_t index;
    entityid_t entity;
    SensorParams params;
    SensorParams calculated;
    std::set<entityid_t> prevEntered;
    std::set<entityid_t> nextEntered;
    sensorcallback enterCallback;
    sensorcallback exitCallback;
};

enum class BodyType {
    Static,
    Kinematic,
    Dynamic
};

CHROMA_ENUM_METADATA(BodyType)
    {"static", BodyType::Static},
    {"kinematic", BodyType::Kinematic},
    {"dynamic", BodyType::Dynamic},
CHROMA_ENUM_END

/**
 * @brief Класс, представляющий физический хитбокс объекта.
 *
 * Хитбокс определяется центром (position), половинами размеров (halfsize),
 * скоростью (velocity), коэффициентом линейного затухания (linear_damping)
 * и флагом grounded, указывающим, касается ли хитбокс земли.
 */
struct Hitbox {
    entityid_t entity;
    BodyType type;
    glm::vec3 position; ///< Центр хитбокса в мировых координатах
    glm::vec3 halfsize; ///< Половины размеров хитбокса по осям X, Y, Z
    glm::vec3 velocity; ///< Текущая скорость хитбокса
    glm::vec3 scale {1.0f, 1.0f, 1.0f};
    float linearDamping = 0.5f; ///< Коэффициент линейного затухания скорости
    float friction = 1.0f;
    float verticalDamping = 1.0f;
    bool grounded = false; ///< Флаг, указывающий, находится ли хитбокс на земле
    float gravityScale = 1.0f;
    bool crouching = false;
    float stepHeight = 0.5f;
    float mass = 1.0f;
    float elasticity = 0.0f;
    std::string material;

    std::string groundMaterial;
    glm::vec3 groundVelocity {};

    glm::vec3 prevPosition {};
    glm::vec3 prevVelocity {};
    bool prevGrounded = false;

    static inline constexpr float TELEPORT_THRESOLD_SQR = 0.5f;

    Hitbox(
        entityid_t entity, BodyType type, glm::vec3 position, glm::vec3 halfsize
    );

    AABB getAABB() const {
        return AABB(position - halfsize, position + halfsize);
    }

    glm::vec3 getHalfSize() const {
        return halfsize * scale;
    }

    void setPos(const glm::vec3& vec) {
        position = vec;
        if (glm::distance2(position, prevPosition) >= TELEPORT_THRESOLD_SQR) {
            prevPosition = vec;
        }
    }
};
