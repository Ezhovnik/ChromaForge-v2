#ifndef PHYSICS_HITBOX_H_
#define PHYSICS_HITBOX_H_

#include <set>
#include <functional>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../math/AABB.h"
#include "../typedefs.h"

enum class TriggerType {
    AABB,
    RADIUS,
};

union TriggerParams {
    AABB aabb;
    glm::vec4 radial;

    constexpr TriggerParams() : aabb() {
    }
};

using triggercallback = std::function<void(entityid_t, size_t, entityid_t)>;

struct Trigger {
    bool enabled = true;
    TriggerType type;
    size_t index;
    entityid_t entity;
    TriggerParams params;
    TriggerParams calculated;
    std::set<entityid_t> prevEntered;
    std::set<entityid_t> nextEntered;
    triggercallback enterCallback;
    triggercallback exitCallback;
};

/**
 * @brief Класс, представляющий физический хитбокс объекта.
 *
 * Хитбокс определяется центром (position), половинами размеров (halfsize),
 * скоростью (velocity), коэффициентом линейного затухания (linear_damping)
 * и флагом grounded, указывающим, касается ли хитбокс земли.
 */
struct Hitbox {
	glm::vec3 position; ///< Центр хитбокса в мировых координатах
	glm::vec3 halfsize; ///< Половины размеров хитбокса по осям X, Y, Z
	glm::vec3 velocity; ///< Текущая скорость хитбокса
    float linearDamping; ///< Коэффициент линейного затухания скорости
	bool grounded = false; ///< Флаг, указывающий, находится ли хитбокс на земле

	/**
     * @brief Конструктор хитбокса.
     * @param position Начальная позиция центра.
     * @param halfsize Половины размеров по осям.
	 */
	Hitbox(glm::vec3 position, glm::vec3 halfsize);
};

#endif // PHYSICS_HITBOX_H_
