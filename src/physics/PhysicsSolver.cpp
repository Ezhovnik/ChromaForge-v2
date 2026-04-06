#include "PhysicsSolver.h"

#include "Hitbox.h"
#include "../voxels/Chunks.h"
#include "../math/aabb.h"
#include "../voxels/Block.h"
#include "../voxels/voxel.h"

namespace PhysicsSolver_Consts {
    inline constexpr float EPS = 0.03f; // Маленькое значение
	inline constexpr float OVERLAP_EPS = 0.00001f;
    inline constexpr float GROUND_FRICTION = 18.0f; // Коэффициент трения о землю
	inline constexpr float MAX_FIX = 0.1f;
}

// Конструктор
PhysicsSolver::PhysicsSolver(glm::vec3 gravity) : gravity(gravity) {
}

// Выполняет один шаг физического моделирования для хитбокса
void PhysicsSolver::step(
	Chunks* chunks,
	Hitbox* hitbox,
	float delta,
	uint substeps,
	bool shifting,
	float gravityScale,
	bool collisions
) {
	float subDelta = delta / static_cast<float>(substeps);
	float linearDamping = hitbox->linearDamping;
	float step_size = 2.0f / BLOCK_AABB_GRID;

	const glm::vec3& half = hitbox->halfsize;
    glm::vec3& pos = hitbox->position;
    glm::vec3& vel = hitbox->velocity;

	bool prevGrounded = hitbox->grounded;
	hitbox->grounded = false;
    // Разбиваем шаг на подшаги
    for (uint i = 0; i < substeps; ++i) {
		float prev_x = pos.x;
		float prev_y = pos.y;
		float prev_z = pos.z;

		vel += gravity * subDelta * gravityScale;

		if (collisions) colisionCalc(chunks, hitbox, vel, pos, half, (prevGrounded && gravityScale > 0.0f) ? 0.5f : 0.0f);

		vel.x *= glm::max(0.0f, 1.0f - subDelta * linearDamping);
		vel.z *= glm::max(0.0f, 1.0f - subDelta * linearDamping);
		pos += vel * subDelta + gravity * gravityScale * subDelta * subDelta * 0.5f;
		if (hitbox->grounded && pos.y < prev_y) pos.y = prev_y;

		if (shifting && hitbox->grounded){
			float y = pos.y - half.y - PhysicsSolver_Consts::EPS;
			hitbox->grounded = false;
			for (float x = prev_x - half.x + PhysicsSolver_Consts::EPS; x <= prev_x + half.x - PhysicsSolver_Consts::EPS; x += step_size) {
				for (float z = pos.z - half.z + PhysicsSolver_Consts::EPS; z <= pos.z + half.z - PhysicsSolver_Consts::EPS; z += step_size){
					if (chunks->isObstacleAt(x, y, z)){
						hitbox->grounded = true;
						break;
					}
				}
			}
			if (!hitbox->grounded) pos.z = prev_z;

			hitbox->grounded = false;
			for (float x = pos.x - half.x + PhysicsSolver_Consts::EPS; x <= pos.x + half.x - PhysicsSolver_Consts::EPS; x += step_size){
				for (float z = prev_z - half.z + PhysicsSolver_Consts::EPS; z <= prev_z + half.z - PhysicsSolver_Consts::EPS; z += step_size){
					if (chunks->isObstacleAt(x, y, z)){
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

void PhysicsSolver::colisionCalc(
	Chunks* chunks,
	Hitbox* hitbox,
	glm::vec3& vel,
	glm::vec3& pos,
	const glm::vec3& half,
	float stepHeight
) {
	float step_size = 2.0f / BLOCK_AABB_GRID;

	if (stepHeight > 0.0f) {
        for (float x = (pos.x - half.x + PhysicsSolver_Consts::EPS); x <= (pos.x + half.x - PhysicsSolver_Consts::EPS); x += step_size){
            for (float z = (pos.z - half.z + PhysicsSolver_Consts::EPS); z <= (pos.z + half.z - PhysicsSolver_Consts::EPS); z += step_size){
                if (chunks->isObstacleAt(x, pos.y + half.y + stepHeight, z)) {
                    stepHeight = 0.0f;
                    break;
                }
            }
        }
    }

	const AABB* aabb;

	if (vel.x < 0.0f){
		for (float y = pos.y - half.y + PhysicsSolver_Consts::EPS + stepHeight; y <= pos.y + half.y - PhysicsSolver_Consts::EPS; y += step_size){
			for (float z = pos.z - half.z + PhysicsSolver_Consts::EPS; z <= pos.z + half.z - PhysicsSolver_Consts::EPS; z += step_size){
				float x = pos.x - half.x - PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacleAt(x, y, z))){
					vel.x = 0.0f;
					float newx = floor(x) + aabb->max().x + half.x + PhysicsSolver_Consts::EPS;
					if (glm::abs(newx - pos.x) <= PhysicsSolver_Consts::MAX_FIX) pos.x = newx;
					break;
				}
			}
		}
	}
	if (vel.x > 0.0f){
		for (float y = pos.y - half.y + PhysicsSolver_Consts::EPS + stepHeight; y <= pos.y + half.y - PhysicsSolver_Consts::EPS; y += step_size){
			for (float z = pos.z - half.z + PhysicsSolver_Consts::EPS; z <= pos.z + half.z - PhysicsSolver_Consts::EPS; z += step_size){
				float x = pos.x + half.x + PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacleAt(x, y, z))){
					vel.x = 0.0f;
					float newx = floor(x) - half.x + aabb->min().x - PhysicsSolver_Consts::EPS;
					if (glm::abs(newx - pos.x) <= PhysicsSolver_Consts::MAX_FIX) pos.x = newx;
					break;
				}
			}
		}
	}

	if (vel.z < 0.0f){
		for (float y = pos.y - half.y + PhysicsSolver_Consts::EPS + stepHeight; y <= pos.y + half.y - PhysicsSolver_Consts::EPS; y += step_size){
			for (float x = pos.x - half.x + PhysicsSolver_Consts::EPS; x <= pos.x + half.x - PhysicsSolver_Consts::EPS; x += step_size){
				float z = pos.z - half.z - PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacleAt(x, y, z))){
					vel.z = 0.0f;
					float newz = floor(z) + aabb->max().z + half.z + PhysicsSolver_Consts::EPS;
					if (glm::abs(newz - pos.z) <= PhysicsSolver_Consts::MAX_FIX) pos.z = newz;
					break;
				}
			}
		}
	}

	if (vel.z > 0.0f){
		for (float y = pos.y - half.y + PhysicsSolver_Consts::EPS + stepHeight; y <= pos.y + half.y - PhysicsSolver_Consts::EPS; y += step_size){
			for (float x = pos.x - half.x + PhysicsSolver_Consts::EPS; x <= pos.x + half.x - PhysicsSolver_Consts::EPS; x += step_size){
				float z = pos.z + half.z + PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacleAt(x, y, z))){
					vel.z = 0.0f;
					float newz = floor(z) - half.z + aabb->min().z - PhysicsSolver_Consts::EPS;
					if (glm::abs(newz - pos.z) <= PhysicsSolver_Consts::MAX_FIX) pos.z = newz;
					break;
				}
			}
		}
	}

	if (vel.y < 0.0f){
		for (float x = pos.x - half.x + PhysicsSolver_Consts::EPS; x <= pos.x + half.x - PhysicsSolver_Consts::EPS; x += step_size){
			for (float z = pos.z - half.z + PhysicsSolver_Consts::EPS; z <= pos.z + half.z - PhysicsSolver_Consts::EPS; z += step_size){
				float y = pos.y - half.y - PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacleAt(x, y, z))){
					vel.y = 0.0f;
					float newy = floor(y) + aabb->max().y + half.y;
					if (glm::abs(newy - pos.y) <= PhysicsSolver_Consts::MAX_FIX) pos.y = newy;	
					hitbox->grounded = true;
					break;
				}
			}
		}
	}

	if (stepHeight > 0.0 && vel.y <= 0.0f){
		for (float x = (pos.x - half.x + PhysicsSolver_Consts::EPS); x <= (pos.x + half.x - PhysicsSolver_Consts::EPS); x += step_size){
			for (float z = (pos.z - half.z + PhysicsSolver_Consts::EPS); z <= (pos.z + half.z - PhysicsSolver_Consts::EPS); z += step_size){
				float y = pos.y - half.y + PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacleAt(x, y, z))){
					vel.y = 0.0f;
					float newy = floor(y) + aabb->max().y + half.y;
					if (glm::abs(newy - pos.y) <= PhysicsSolver_Consts::MAX_FIX + stepHeight) pos.y = newy;
					break;
				}
			}
		}
	}

	if (vel.y > 0.0f){
		for (float x = pos.x - half.x + PhysicsSolver_Consts::EPS; x <= pos.x + half.x - PhysicsSolver_Consts::EPS; x += step_size){
			for (float z = pos.z - half.z + PhysicsSolver_Consts::EPS; z <= pos.z + half.z - PhysicsSolver_Consts::EPS; z += step_size){
				float y = pos.y + half.y + PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacleAt(x, y, z))){
					vel.y = 0.0f;
					float newy = floor(y) - half.y + aabb->min().y - PhysicsSolver_Consts::EPS;
					if (glm::abs(newy - pos.y) <= PhysicsSolver_Consts::MAX_FIX) pos.y = newy;
					break;
				}
			}
		}
	}
}

// Проверяет, находится ли блок внутри хитбокса
bool PhysicsSolver::isBlockInside(int x, int y, int z, Hitbox* hitbox) {
	const glm::vec3& pos = hitbox->position;
	const glm::vec3& half = hitbox->halfsize;
	return x >= floor(pos.x - half.x) && x <= floor(pos.x + half.x) &&
			z >= floor(pos.z - half.z) && z <= floor(pos.z + half.z) &&
			y >= floor(pos.y - half.y) && y <= floor(pos.y + half.y);
}

bool PhysicsSolver::isBlockInside(int x, int y, int z, Block* def, blockstate state, Hitbox* hitbox) {
	const glm::vec3& pos = hitbox->position;
	const glm::vec3& half = hitbox->halfsize;
	const auto& boxes = def->rotatable ? def->rt.hitboxes[state.rotation] : def->hitboxes;
	for (const auto& block_hitbox : boxes) {
		glm::vec3 min = block_hitbox.min();
		glm::vec3 max = block_hitbox.max();
		if (min.x < pos.x + half.x - x - PhysicsSolver_Consts::OVERLAP_EPS && max.x > pos.x - half.x - x + PhysicsSolver_Consts::OVERLAP_EPS &&
			min.z < pos.z + half.z - z - PhysicsSolver_Consts::OVERLAP_EPS && max.z > pos.z - half.z - z + PhysicsSolver_Consts::OVERLAP_EPS &&
			min.y < pos.y + half.y - y - PhysicsSolver_Consts::OVERLAP_EPS && max.y > pos.y - half.y - y + PhysicsSolver_Consts::OVERLAP_EPS)
			return true;
	}
	return false;
}
