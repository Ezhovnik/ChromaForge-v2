#include "PhysicsSolver.h"

#include "Hitbox.h"
#include "../voxels/Chunks.h"
#include "../math/aabb.h"
#include "../voxels/Block.h"

namespace PhysicsSolver_Consts {
    inline constexpr float EPS = 0.03; // Маленькое значение
    inline constexpr float GROUND_FRICTION = 18.0f; // Коэффициент трения о землю
}

// Конструктор
PhysicsSolver::PhysicsSolver(glm::vec3 gravity) : gravity(gravity) {
}

// Выполняет один шаг физического моделирования для хитбокса
void PhysicsSolver::step(Chunks* chunks, Hitbox* hitbox, float delta, uint substeps, bool shifting, float gravityScale, bool collisions) {
	float subDelta = delta / float(substeps);
	float linear_damping = hitbox->linear_damping;
	float step_size = 2.0f / BLOCK_AABB_GRID;
	
	hitbox->grounded = false;
    // Разбиваем шаг на подшаги
    for (uint i = 0; i < substeps; ++i) {
		glm::vec3& pos = hitbox->position;
		glm::vec3& half = hitbox->halfsize;
		glm::vec3& vel = hitbox->velocity;
		float prev_x = pos.x;
		float prev_z = pos.z;
		
		vel += gravity * subDelta * gravityScale;

		if (collisions) colisionCalc(chunks, hitbox, vel, pos, half);

		vel.x *= glm::max(0.0f, 1.0f - subDelta * linear_damping);
		vel.z *= glm::max(0.0f, 1.0f - subDelta * linear_damping);
		pos += vel * subDelta;

		if (shifting && hitbox->grounded){
			float y = pos.y - half.y - PhysicsSolver_Consts::EPS;
			hitbox->grounded = false;
			for (float x = prev_x - half.x + PhysicsSolver_Consts::EPS; x <= prev_x + half.x - PhysicsSolver_Consts::EPS; x += step_size) {
				for (float z = pos.z - half.z + PhysicsSolver_Consts::EPS; z <= pos.z + half.z - PhysicsSolver_Consts::EPS; z += step_size){
					if (chunks->isObstacle(x, y, z)){
						hitbox->grounded = true;
						break;
					}
				}
			}
			if (!hitbox->grounded) pos.z = prev_z;

			hitbox->grounded = false;
			for (float x = pos.x - half.x + PhysicsSolver_Consts::EPS; x <= pos.x + half.x - PhysicsSolver_Consts::EPS; x += step_size){
				for (float z = prev_z - half.z + PhysicsSolver_Consts::EPS; z <= prev_z + half.z - PhysicsSolver_Consts::EPS; z += step_size){
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

void PhysicsSolver::colisionCalc(Chunks* chunks, Hitbox* hitbox, glm::vec3& vel, glm::vec3& pos, const glm::vec3& half){
	float step_size = 2.0f / BLOCK_AABB_GRID;

	const AABB* aabb;

	if (vel.x < 0.0f){
		for (float y = pos.y - half.y + PhysicsSolver_Consts::EPS; y <= pos.y + half.y - PhysicsSolver_Consts::EPS; y += step_size){
			for (float z = pos.z - half.z + PhysicsSolver_Consts::EPS; z <= pos.z + half.z - PhysicsSolver_Consts::EPS; z += step_size){
				float x = pos.x - half.x - PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacle(x, y, z))){
					vel.x *= 0.0f;
					pos.x = floor(x) + aabb->max().x + half.x + PhysicsSolver_Consts::EPS;
					break;
				}
			}
		}
	}
	if (vel.x > 0.0f){
		for (float y = pos.y - half.y + PhysicsSolver_Consts::EPS; y <= pos.y + half.y - PhysicsSolver_Consts::EPS; y += step_size){
			for (float z = pos.z - half.z + PhysicsSolver_Consts::EPS; z <= pos.z + half.z - PhysicsSolver_Consts::EPS; z += step_size){
				float x = pos.x + half.x + PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacle(x, y, z))){
					vel.x *= 0.0f;
					pos.x = floor(x) - half.x + aabb->min().x - PhysicsSolver_Consts::EPS;
					break;
				}
			}
		}
	}

	if (vel.z < 0.0f){
		for (float y = pos.y - half.y + PhysicsSolver_Consts::EPS; y <= pos.y + half.y - PhysicsSolver_Consts::EPS; y += step_size){
			for (float x = pos.x - half.x + PhysicsSolver_Consts::EPS; x <= pos.x + half.x - PhysicsSolver_Consts::EPS; x += step_size){
				float z = pos.z - half.z - PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacle(x, y, z))){
					vel.z *= 0.0f;
					pos.z = floor(z) + aabb->max().z + half.z + PhysicsSolver_Consts::EPS;
					break;
				}
			}
		}
	}

	if (vel.z > 0.0f){
		for (float y = pos.y - half.y + PhysicsSolver_Consts::EPS; y <= pos.y + half.y - PhysicsSolver_Consts::EPS; y += step_size){
			for (float x = pos.x - half.x + PhysicsSolver_Consts::EPS; x <= pos.x + half.x - PhysicsSolver_Consts::EPS; x += step_size){
				float z = pos.z + half.z + PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacle(x, y, z))){
					vel.z *= 0.0f;
					pos.z = floor(z) - half.z + aabb->min().z - PhysicsSolver_Consts::EPS;
					break;
				}
			}
		}
	}

	if (vel.y < 0.0f){
		for (float x = pos.x - half.x + PhysicsSolver_Consts::EPS; x <= pos.x + half.x - PhysicsSolver_Consts::EPS; x += step_size){
			for (float z = pos.z - half.z + PhysicsSolver_Consts::EPS; z <= pos.z + half.z - PhysicsSolver_Consts::EPS; z += step_size){
				float y = pos.y - half.y - PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacle(x, y, z))){
					vel.y *= 0.0f;
					pos.y = floor(y) + aabb->max().y + half.y;
					hitbox->grounded = true;
					break;
				}
			}
		}
	}
	if (vel.y > 0.0f){
		for (float x = pos.x - half.x + PhysicsSolver_Consts::EPS; x <= pos.x + half.x - PhysicsSolver_Consts::EPS; x += step_size){
			for (float z = pos.z - half.z + PhysicsSolver_Consts::EPS; z <= pos.z + half.z - PhysicsSolver_Consts::EPS; z += step_size){
				float y = pos.y + half.y + PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacle(x, y, z))){
					vel.y *= 0.0f;
					pos.y = floor(y) - half.y + aabb->min().y - PhysicsSolver_Consts::EPS;
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
