#ifndef PHYSICS_HITBOX_H_
#define PHYSICS_HITBOX_H_

#include <set>
#include <functional>
#include <string>
#include <optional>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../math/AABB.h"
#include "../typedefs.h"

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
    Kinematic,
    Dynamic
};

std::optional<BodyType> BodyType_from(std::string_view str);
std::string to_string(BodyType type);

/**
 * @brief Класс, представляющий физический хитбокс объекта.
 *
 * Хитбокс определяется центром (position), половинами размеров (halfsize),
 * скоростью (velocity), коэффициентом линейного затухания (linear_damping)
 * и флагом grounded, указывающим, касается ли хитбокс земли.
 */
struct Hitbox {
    BodyType type;
	glm::vec3 position; ///< Центр хитбокса в мировых координатах
	glm::vec3 halfsize; ///< Половины размеров хитбокса по осям X, Y, Z
	glm::vec3 velocity; ///< Текущая скорость хитбокса
    float linearDamping; ///< Коэффициент линейного затухания скорости
    bool verticalDamping = false;
	bool grounded = false; ///< Флаг, указывающий, находится ли хитбокс на земле
    float gravityScale = 1.0f;
    bool crouching = false;

	/**
     * @brief Конструктор хитбокса.
     * @param type Тип тела для расчета физики.
     * @param position Начальная позиция центра.
     * @param halfsize Половины размеров по осям.
	 */
	Hitbox(BodyType type, glm::vec3 position, glm::vec3 halfsize);
};

#endif // PHYSICS_HITBOX_H_
