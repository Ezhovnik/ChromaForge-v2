#include "PhysicsSolver.h"
#include "Hitbox.h"
#include "../voxels/Chunks.h"

#include <iostream>

namespace PhysicsSolver_Consts {
    inline constexpr float EPS = 0.03; // Маленькое значение
    inline constexpr float GROUND_FRICTION = 18.0f; // Коэффициент трения о землю
}

// Конструктор
PhysicsSolver::PhysicsSolver(glm::vec3 gravity) : gravity(gravity) {
}

// Выполняет один шаг физического моделирования для хитбокса
void PhysicsSolver::step(Chunks* chunks, Hitbox* hitbox, float delta, unsigned substeps, bool shifting, float gravityScale, bool collisions) {
	hitbox->grounded = false;
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

        if (collisions) {
            colisionCalc(chunks, hitbox, &vel, &pos, half);
        }

        vel.x *= glm::max(0.0, 1.0 - subDelta * linear_damping);
		vel.z *= glm::max(0.0, 1.0 - subDelta * linear_damping);

        // Обновляем позицию на основе скорости
		pos.x += vel.x * subDelta;
		pos.y += vel.y * subDelta;
		pos.z += vel.z * subDelta;

        // Проверка сдвига при движении по земле
		if (shifting && hitbox->grounded){
			int y = floor(pos.y - half.y - PhysicsSolver_Consts::EPS);

			hitbox->grounded = false;
			for (int x = floor(prev_x - half.x + PhysicsSolver_Consts::EPS); x <= floor(prev_x + half.x - PhysicsSolver_Consts::EPS); ++x){
				for (int z = floor(pos.z - half.z + PhysicsSolver_Consts::EPS); z <= floor(pos.z + half.z - PhysicsSolver_Consts::EPS); ++z){
					if (chunks->isObstacle(x, y, z)){
						hitbox->grounded = true;
						break;
					}
				}
			}
			if (!hitbox->grounded) pos.z = prev_z;

			hitbox->grounded = false;

			for (int x = floor(pos.x - half.x + PhysicsSolver_Consts::EPS); x <= floor(pos.x + half.x - PhysicsSolver_Consts::EPS); ++x){
				for (int z = floor(prev_z - half.z + PhysicsSolver_Consts::EPS); z <= floor(prev_z + half.z - PhysicsSolver_Consts::EPS); ++z){
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

void PhysicsSolver::colisionCalc(Chunks* chunks, Hitbox* hitbox, glm::vec3* vel, glm::vec3* pos, glm::vec3 half){
	if (vel->x < 0.0){
		for (int y = floor(pos->y-half.y+PhysicsSolver_Consts::EPS); y <= floor(pos->y+half.y-PhysicsSolver_Consts::EPS); ++y){
			for (int z = floor(pos->z-half.z+PhysicsSolver_Consts::EPS); z <= floor(pos->z+half.z-PhysicsSolver_Consts::EPS); ++z){
				int x = floor(pos->x-half.x-PhysicsSolver_Consts::EPS);
				if (chunks->isObstacle(x, y, z)){
					vel->x *= 0.0;
					pos->x = x + 1 + half.x + PhysicsSolver_Consts::EPS;
					break;
				}
			}
		}
	}
	if (vel->x > 0.0){
		for (int y = floor(pos->y-half.y+PhysicsSolver_Consts::EPS); y <= floor(pos->y+half.y-PhysicsSolver_Consts::EPS); y++){
			for (int z = floor(pos->z-half.z+PhysicsSolver_Consts::EPS); z <= floor(pos->z+half.z-PhysicsSolver_Consts::EPS); z++){
				int x = floor(pos->x+half.x+PhysicsSolver_Consts::EPS);
				if (chunks->isObstacle(x,y,z)){
					vel->x *= 0.0;
					pos->x = x - half.x - PhysicsSolver_Consts::EPS;
					break;
				}
			}
		}
	}

	if (vel->z < 0.0){
		for (int y = floor(pos->y-half.y+PhysicsSolver_Consts::EPS); y <= floor(pos->y+half.y-PhysicsSolver_Consts::EPS); y++){
			for (int x = floor(pos->x-half.x+PhysicsSolver_Consts::EPS); x <= floor(pos->x+half.x-PhysicsSolver_Consts::EPS); x++){
				int z = floor(pos->z-half.z-PhysicsSolver_Consts::EPS);
				if (chunks->isObstacle(x,y,z)){
					vel->z *= 0.0;
					pos->z = z + 1 + half.z + PhysicsSolver_Consts::EPS;
					break;
				}
			}
		}
	}

	if (vel->z > 0.0){
		for (int y = floor(pos->y-half.y+PhysicsSolver_Consts::EPS); y <= floor(pos->y+half.y-PhysicsSolver_Consts::EPS); y++){
			for (int x = floor(pos->x-half.x+PhysicsSolver_Consts::EPS); x <= floor(pos->x+half.x-PhysicsSolver_Consts::EPS); x++){
				int z = floor(pos->z+half.z+PhysicsSolver_Consts::EPS);
				if (chunks->isObstacle(x,y,z)){
					vel->z *= 0.0;
					pos->z = z - half.z - PhysicsSolver_Consts::EPS;
					break;
				}
			}
		}
	}

	if (vel->y < 0.0){
		for (int x = floor(pos->x-half.x+PhysicsSolver_Consts::EPS); x <= floor(pos->x+half.x-PhysicsSolver_Consts::EPS); x++){
			for (int z = floor(pos->z-half.z+PhysicsSolver_Consts::EPS); z <= floor(pos->z+half.z-PhysicsSolver_Consts::EPS); z++){
				int y = floor(pos->y-half.y-PhysicsSolver_Consts::EPS);
				if (chunks->isObstacle(x,y,z)){
					vel->y *= 0.0;
					pos->y = y + 1 + half.y;
					hitbox->grounded = true;
					break;
				}
			}
		}
	}
	if (vel->y > 0.0){
		for (int x = floor(pos->x-half.x+PhysicsSolver_Consts::EPS); x <= floor(pos->x+half.x-PhysicsSolver_Consts::EPS); x++){
			for (int z = floor(pos->z-half.z+PhysicsSolver_Consts::EPS); z <= floor(pos->z+half.z-PhysicsSolver_Consts::EPS); z++){
				int y = floor(pos->y+half.y+PhysicsSolver_Consts::EPS);
				if (chunks->isObstacle(x,y,z)){
					vel->y *= 0.0;
					pos->y = y - half.y - PhysicsSolver_Consts::EPS;
					break;
				}
			}
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
