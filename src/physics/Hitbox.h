#ifndef PHYSICS_HITBOX_H_
#define PHYSICS_HITBOX_H_

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
