#ifndef PHYSICS_PHYSICSSOLVER_H_
#define PHYSICS_PHYSICSSOLVER_H_

#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "typedefs.h"
#include "Hitbox.h"

class Chunks;
class Block;
struct blockstate;

// Класс для решения физических взаимодействий объектов с воксельным миром.
class PhysicsSolver {
private:
	glm::vec3 gravity; // Вектор гравитации, применяемой к объектам
	std::vector<Sensor*> sensors;
public:
	PhysicsSolver(glm::vec3 gravity); // Конструтор

	void step(
		Chunks* chunks,
		Hitbox* hitbox,
		float delta,
		uint substeps,
        entityid_t entity
	); // Выполняет один шаг физического моделирования для указанного хитбокса.

	void colisionCalc(
		Chunks* chunks,
		Hitbox* hitbox,
		glm::vec3& vel,
		glm::vec3& pos,
		const glm::vec3& half,
		float stepHeight
	);
    bool isBlockInside(int x, int y, int z, Hitbox* hitbox); // Проверяет, находится ли указанный блок внутри границ хитбокса.
	bool isBlockInside(
		int x, int y, int z,
		Block* def,
		blockstate state,
		Hitbox* hitbox
	);

	void setSensors(std::vector<Sensor*> sensors) {
        this->sensors = std::move(sensors);
    }
};

#endif // PHYSICS_PHYSICSSOLVER_H_
