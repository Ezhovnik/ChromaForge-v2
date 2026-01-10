#ifndef PHYSICS_PHYSICSSOLVER_H_
#define PHYSICS_PHYSICSSOLVER_H_

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Hitbox;
class Chunks;

// Класс для решения физических взаимодействий объектов с воксельным миром.
class PhysicsSolver {
	glm::vec3 gravity; // Вектор гравитации, применяемой к объектам
public:
	PhysicsSolver(glm::vec3 gravity); // Конструтор

	void step(Chunks* chunks, Hitbox* hitbox, float delta, unsigned substeps, bool shifting, float gravityScale, bool collisions); // Выполняет один шаг физического моделирования для указанного хитбокса.
	void colisionCalc(Chunks* chunks, Hitbox* hitbox, glm::vec3* vel, glm::vec3* pos, glm::vec3 half);
    bool isBlockInside(int x, int y, int z, Hitbox* hitbox); // Проверяет, находится ли указанный блок внутри границ хитбокса.
};

#endif // PHYSICS_PHYSICSSOLVER_H_
