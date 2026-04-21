#include <physics/PhysicsSolver.h>

#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <physics/Hitbox.h>
#include <voxels/Chunks.h>
#include <math/AABB.h>
#include <voxels/Block.h>
#include <voxels/voxel.h>

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
    entityid_t entity
) {
	float subDelta = delta / static_cast<float>(substeps);
	float linearDamping = hitbox->linearDamping;
	float step_size = 2.0f / BLOCK_AABB_GRID;
	float gravityScale = hitbox->gravityScale;

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

		if (hitbox->type == BodyType::Dynamic) {
			colisionCalc(
				chunks,
				hitbox,
				vel,
				pos,
				half,
				(prevGrounded && gravityScale > 0.0f) ? 0.5f : 0.0f
			);
		}

		vel.x *= glm::max(0.0f, 1.0f - subDelta * linearDamping);
		if (hitbox->verticalDamping) {
            vel.y *= glm::max(0.0f, 1.0f - subDelta * linearDamping);
        }
		vel.z *= glm::max(0.0f, 1.0f - subDelta * linearDamping);
		pos += vel * subDelta + gravity * gravityScale * subDelta * subDelta * 0.5f;
		if (hitbox->grounded && pos.y < prev_y) pos.y = prev_y;

		if (hitbox->crouching && hitbox->grounded){
			float y = pos.y - half.y - PhysicsSolver_Consts::EPS;
			hitbox->grounded = false;
			// for (float x = prev_x - half.x + PhysicsSolver_Consts::EPS; x <= prev_x + half.x - PhysicsSolver_Consts::EPS; x += step_size) {
			// 	for (float z = pos.z - half.z + PhysicsSolver_Consts::EPS; z <= pos.z + half.z - PhysicsSolver_Consts::EPS; z += step_size){
			for (int ix = 0; ix <= (half.x - PhysicsSolver_Consts::EPS) * 2 / step_size; ++ix) {
                float x = (prev_x - half.x + PhysicsSolver_Consts::EPS) + ix * step_size;
                for (int iz = 0; iz <= (half.z - PhysicsSolver_Consts::EPS) * 2 / step_size; ++iz) {
                    float z = (pos.z - half.z + PhysicsSolver_Consts::EPS) + iz * step_size;
					if (chunks->isObstacleAt(x, y, z)){
						hitbox->grounded = true;
						break;
					}
				}
			}
			if (!hitbox->grounded) pos.z = prev_z;

			hitbox->grounded = false;
			for (int ix = 0; ix <= (half.x - PhysicsSolver_Consts::EPS) * 2 / step_size; ++ix) {
                float x = (pos.x - half.x + PhysicsSolver_Consts::EPS) + ix * step_size;
                for (int iz = 0; iz <= (half.z - PhysicsSolver_Consts::EPS) * 2 / step_size; ++iz) {
                    float z = (prev_z - half.z + PhysicsSolver_Consts::EPS) + iz * step_size;
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

	AABB aabb;
    aabb.a = hitbox->position - hitbox->halfsize;
    aabb.b = hitbox->position + hitbox->halfsize;
    for (size_t i = 0; i < sensors.size(); ++i) {
        auto& sensor = *sensors[i];
        if (sensor.entity == entity) continue;
        bool sensored = false;
        switch (sensor.type) {
            case SensorType::AABB:
                sensored = aabb.intersect(sensor.calculated.aabb);
                break;
            case SensorType::RADIUS:
                sensored = glm::distance2(
                    hitbox->position, glm::vec3(sensor.calculated.radial)
				) < sensor.calculated.radial.w;
                break;
        }
        if (sensored) {
            if (sensor.prevEntered.find(entity) == sensor.prevEntered.end()) {
                sensor.enterCallback(sensor.entity, sensor.index, entity);
            }
            sensor.nextEntered.insert(entity);
        }
    }
}

static float calc_step_height(
	Chunks* chunks,
	glm::vec3& pos,
	const glm::vec3& half,
    float stepHeight,
    float step_size
) {
	if (stepHeight > 0.0f) {
        for (int ix = 0; ix <= (half.x - PhysicsSolver_Consts::EPS) * 2 / step_size; ++ix) {
            float x = (pos.x - half.x + PhysicsSolver_Consts::EPS) + ix * step_size;
            for (int iz = 0; iz <= (half.z - PhysicsSolver_Consts::EPS) * 2 / step_size; ++iz) {
                float z = (pos.z - half.z + PhysicsSolver_Consts::EPS) + iz * step_size;
                if (chunks->isObstacleAt(x, pos.y + half.y + stepHeight, z)) {
                    return 0.0f;
                }
            }
        }
    }
	return stepHeight;
}

template<int nx, int ny, int nz>
static bool calc_collision_neg(
    Chunks* chunks,
    glm::vec3& pos,
    glm::vec3& vel,
    const glm::vec3& half,
    float stepHeight,
    float step_size
) {
    if (vel[nx] >= 0.0f) return false;

	glm::vec3 offset(0.0f, stepHeight, 0.0f);
    for (int iy = 0; iy <= (half[ny] - PhysicsSolver_Consts::EPS) * 2 / step_size; ++iy) {
        glm::vec3 coord;
        coord[ny] = ((pos + offset)[ny] - half[ny] + PhysicsSolver_Consts::EPS) + iy * step_size;
        for (int iz = 0; iz <= (half[nz] - PhysicsSolver_Consts::EPS) * 2 / step_size; ++iz){
            coord[nz] = (pos[nz] - half[nz] + PhysicsSolver_Consts::EPS) + iz * step_size;
            coord[nx] = (pos[nx] - half[nx] - PhysicsSolver_Consts::EPS);

			if (const auto aabb = chunks->isObstacleAt(coord.x, coord.y, coord.z)) {
                vel[nx] = 0.0f;
                float newx = std::floor(coord[nx]) + aabb->max()[nx] + half[nx] + PhysicsSolver_Consts::EPS;
                if (std::abs(newx-pos[nx]) <= PhysicsSolver_Consts::MAX_FIX) {
                    pos[nx] = newx;
				}
				return true;
			}
		}
	}
	return false;
}

template <int nx, int ny, int nz>
static void calc_collision_pos(
    Chunks* chunks,
    glm::vec3& pos,
    glm::vec3& vel,
    const glm::vec3& half,
    float stepHeight,
    float step_size
) {
    if (vel[nx] <= 0.0f) return;

    glm::vec3 offset(0.0f, stepHeight, 0.0f);
    for (int iy = 0; iy <= (half[ny] - PhysicsSolver_Consts::EPS) * 2 / step_size; ++iy) {
        glm::vec3 coord;
        coord[ny] = ((pos + offset)[ny] - half[ny] + PhysicsSolver_Consts::EPS) + iy * step_size;
        for (int iz = 0; iz <= (half[nz] - PhysicsSolver_Consts::EPS) * 2 / step_size; ++iz) {
            coord[nz] = (pos[nz] - half[nz] + PhysicsSolver_Consts::EPS) + iz * step_size;
            coord[nx] = (pos[nx] + half[nx] + PhysicsSolver_Consts::EPS);
            if (const auto aabb = chunks->isObstacleAt(coord.x, coord.y, coord.z)) {
                vel[nx] = 0.0f;
                float newx = std::floor(coord[nx]) - half[nx] + aabb->min()[nx] - PhysicsSolver_Consts::EPS;
                if (std::abs(newx - pos[nx]) <= PhysicsSolver_Consts::MAX_FIX) {
                    pos[nx] = newx;
				}
				return;
			}
		}
	}
}

void PhysicsSolver::colisionCalc(
    Chunks* chunks, 
    Hitbox* hitbox, 
    glm::vec3& vel, 
    glm::vec3& pos, 
    const glm::vec3 half,
    float stepHeight
) {
    float step_size = 2.0f / BLOCK_AABB_GRID;

    stepHeight = calc_step_height(chunks, pos, half, stepHeight, step_size);

    const AABB* aabb;

    calc_collision_neg<0, 1, 2>(chunks, pos, vel, half, stepHeight, step_size);
    calc_collision_pos<0, 1, 2>(chunks, pos, vel, half, stepHeight, step_size);

    calc_collision_neg<2, 1, 0>(chunks, pos, vel, half, stepHeight, step_size);
    calc_collision_pos<2, 1, 0>(chunks, pos, vel, half, stepHeight, step_size);

    if (calc_collision_neg<1, 0, 2>(chunks, pos, vel, half, stepHeight, step_size)) {
        hitbox->grounded = true;
	}

	if (stepHeight > 0.0 && vel.y <= 0.0f){
		for (int ix = 0; ix <= (half.x - PhysicsSolver_Consts::EPS) * 2 / step_size; ++ix) {
            float x = (pos.x - half.x + PhysicsSolver_Consts::EPS) + ix * step_size;
            for (int iz = 0; iz <= (half.z - PhysicsSolver_Consts::EPS) * 2 / step_size; ++iz) {
                float z = (pos.z - half.z + PhysicsSolver_Consts::EPS) + iz * step_size;
				float y = pos.y - half.y + PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacleAt(x, y, z))){
					vel.y = 0.0f;
					float newy = std::floor(y) + aabb->max().y + half.y;
					if (std::abs(newy - pos.y) <= PhysicsSolver_Consts::MAX_FIX + stepHeight) pos.y = newy;
					break;
				}
			}
		}
	}

	if (vel.y > 0.0f){
		for (int ix = 0; ix <= (half.x - PhysicsSolver_Consts::EPS) * 2 / step_size; ++ix) {
            float x = (pos.x - half.x + PhysicsSolver_Consts::EPS) + ix * step_size;
            for (int iz = 0; iz <= (half.z-PhysicsSolver_Consts::EPS) * 2 / step_size; ++iz) {
                float z = (pos.z - half.z + PhysicsSolver_Consts::EPS) + iz * step_size;
				float y = pos.y + half.y + PhysicsSolver_Consts::EPS;
				if ((aabb = chunks->isObstacleAt(x, y, z))){
					vel.y = 0.0f;
					float newy = std::floor(y) - half.y + aabb->min().y - PhysicsSolver_Consts::EPS;
					if (std::abs(newy - pos.y) <= PhysicsSolver_Consts::MAX_FIX) pos.y = newy;
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

void PhysicsSolver::removeSensor(Sensor* sensor) {
    sensors.erase(std::remove(sensors.begin(), sensors.end(), sensor), sensors.end());
}
