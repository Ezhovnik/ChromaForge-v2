#include "PhysicsSolver.h"
#include "Hitbox.h"
#include "../voxels/Chunks.h"

#include <iostream>

constexpr float EPS = 0.01; // Маленькое значение
constexpr float GROUND_FRICTION = 18.0f; // Коэффициент трения о землю

// Конструктор
PhysicsSolver::PhysicsSolver(glm::vec3 gravity) : gravity(gravity) {
}

// Выполняет один шаг физического моделирования для хитбокса
void PhysicsSolver::step(Chunks* chunks, Hitbox* hitbox, float delta, unsigned substeps, bool shifting, float gravityScale) {
	// Разбиваем шаг на подшаги
    for (unsigned i = 0; i < substeps; ++i){
		float subDelta = delta / (float)substeps;
        float linear_damping = hitbox->linear_damping;
    
		glm::vec3& pos = hitbox->position;
		glm::vec3& half = hitbox->halfsize;
		glm::vec3& vel = hitbox->velocity;

		vel.x += gravity.x * subDelta * gravityScale;
		vel.y += gravity.y * subDelta * gravityScale;
		vel.z += gravity.z * subDelta * gravityScale;

		float prev_x = pos.x;
		float prev_z = pos.z;

        // Обработка столкновений при движении влево (-X)
		if (vel.x < 0.0){
			for (int y = floor(pos.y - half.y + EPS); y <= floor(pos.y + half.y - EPS); ++y){
				for (int z = floor(pos.z - half.z + EPS); z <= floor(pos.z + half.z - EPS); ++z){
					int x = floor(pos.x - half.x - EPS);
					if (chunks->isObstacle(x, y, z)){
						vel.x *= 0.0;
						pos.x = x + 1 + half.x + EPS;
						break;
					}
				}
			}
		}
        // Обработка столкновений при движении вправо (+X)
		if (vel.x > 0.0){
			for (int y = floor(pos.y - half.y + EPS); y <= floor(pos.y + half.y - EPS); ++y){
				for (int z = floor(pos.z - half.z + EPS); z <= floor(pos.z + half.z - EPS); ++z){
					int x = floor(pos.x + half.x + EPS);
					if (chunks->isObstacle(x, y, z)){
						vel.x *= 0.0;
						pos.x = x - half.x - EPS;
						break;
					}
				}
			}
		}

        // Обработка столкновений при движении назад (-Z)
		if (vel.z < 0.0){
			for (int y = floor(pos.y - half.y + EPS); y <= floor(pos.y + half.y - EPS); ++y){
				for (int x = floor(pos.x - half.x + EPS); x <= floor(pos.x + half.x - EPS); ++x){
					int z = floor(pos.z - half.z - EPS);
					if (chunks->isObstacle(x, y, z)){
						vel.z *= 0.0;
						pos.z = z + 1 + half.z + EPS;
						break;
					}
				}
			}
		}
        // Обработка столкновений при движении вперед (+Z)
		if (vel.z > 0.0){
			for (int y = floor(pos.y - half.y + EPS); y <= floor(pos.y + half.y - EPS); ++y){
				for (int x = floor(pos.x - half.x + EPS); x <= floor(pos.x + half.x - EPS); ++x){
					int z = floor(pos.z + half.z + EPS);
					if (chunks->isObstacle(x, y, z)){
						vel.z *= 0.0;
						pos.z = z - half.z - EPS;
						break;
					}
				}
			}
		}

		hitbox->grounded = false;

        // Падение вниз (-Y)
		if (vel.y < 0.0){
			for (int x = floor(pos.x - half.x + EPS); x <= floor(pos.x + half.x - EPS); ++x){
				bool broken = false;
                for (int z = floor(pos.z - half.z + EPS); z <= floor(pos.z + half.z - EPS); ++z){
					int y = floor(pos.y - half.y - EPS);
					if (chunks->isObstacle(x, y, z)){
						vel.y *= 0.0;
						pos.y = y + 1 + half.y;
						float frictionFactor = glm::max(0.0f, 1.0f - subDelta * GROUND_FRICTION);
						vel.x *= frictionFactor;
						vel.z *= frictionFactor;
						hitbox->grounded = true;
                        broken = true;
						break;
					}
				}
                if (broken) break;
			}
		}
        // Прыжок/подъем вверх (+Y)
		if (vel.y > 0.0){
			for (int x = floor(pos.x - half.x + EPS); x <= floor(pos.x + half.x - EPS); ++x){
				for (int z = floor(pos.z - half.z + EPS); z <= floor(pos.z + half.z - EPS); ++z){
					int y = floor(pos.y + half.y + EPS);
					if (chunks->isObstacle(x, y, z)){
						vel.y *= 0.0;
						pos.y = y - half.y - EPS;
						break;
					}
				}
			}
		}

        vel.x *= glm::max(0.0, 1.0 - subDelta * linear_damping);
		vel.z *= glm::max(0.0, 1.0 - subDelta * linear_damping);

        // Обновляем позицию на основе скорости
		pos.x += vel.x * subDelta;
		pos.y += vel.y * subDelta;
		pos.z += vel.z * subDelta;

        // Проверка сдвига при движении по земле
		if (shifting && hitbox->grounded){
			int y = floor(pos.y - half.y - EPS);

			hitbox->grounded = false;
			for (int x = floor(prev_x - half.x + EPS); x <= floor(prev_x + half.x - EPS); ++x){
				for (int z = floor(pos.z - half.z + EPS); z <= floor(pos.z + half.z - EPS); ++z){
					if (chunks->isObstacle(x, y, z)){
						hitbox->grounded = true;
						break;
					}
				}
			}
			if (!hitbox->grounded) pos.z = prev_z;

			hitbox->grounded = false;

			for (int x = floor(pos.x - half.x + EPS); x <= floor(pos.x + half.x - EPS); ++x){
				for (int z = floor(prev_z - half.z + EPS); z <= floor(prev_z + half.z - EPS); ++z){
					if (chunks->isObstacle(x, y, z)){
						hitbox->grounded = true;
						break;
					}
				}
			}
			if (!hitbox->grounded) pos.x = prev_x;

			hitbox->grounded = true;
		}
	}
}

// Проверяет, находится ли блок внутри хитбокса
bool PhysicsSolver::isBlockInside(int x, int y, int z, Hitbox* hitbox) {
	glm::vec3& pos = hitbox->position;
	glm::vec3& half = hitbox->halfsize;
	return x >= floor(pos.x - half.x) && x <= floor(pos.x + half.x) &&
			z >= floor(pos.z - half.z) && z <= floor(pos.z + half.z) &&
			y >= floor(pos.y - half.y) && y <= floor(pos.y + half.y);
}
