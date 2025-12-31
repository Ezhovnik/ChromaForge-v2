#ifndef PHYSICS_HITBOX_H_
#define PHYSICS_HITBOX_H_

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Hitbox {
public:
	glm::vec3 position; // Центр хитбокса в мировых координатах
	glm::vec3 halfsize; // Половины размеров хитбокса по осям X, Y, Z
	glm::vec3 velocity; // Текущая скорость хитбокса
	bool grounded = false; // Флаг, указывающий, находится ли хитбокс на земле

	Hitbox(glm::vec3 position, glm::vec3 halfsize); // Конструктор
};

#endif // PHYSICS_HITBOX_H_
