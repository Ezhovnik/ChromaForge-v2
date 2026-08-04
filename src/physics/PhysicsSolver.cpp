#include <physics/PhysicsSolver.h>

#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <physics/Hitbox.h>
#include <voxels/GlobalChunks.h>
#include <math/AABB.h>
#include <voxels/Block.h>
#include <voxels/voxel.h>
#include <debug/Logger.h>

namespace PhysicsSolver_Consts {
    inline constexpr float EPS = 0.03f; // Маленькое значение
	inline constexpr float OVERLAP_EPS = 0.00001f;
    inline constexpr float GROUND_FRICTION = 18.0f; // Коэффициент трения о землю
	inline constexpr float MAX_FIX = 0.1f;
}

static debug::Logger logger("physics-solver");

// Конструктор
PhysicsSolver::PhysicsSolver(glm::vec3 gravity) : gravity(gravity) {
}

// Выполняет один шаг физического моделирования для хитбокса
void PhysicsSolver::step(
	const GlobalChunks& chunks,
	Hitbox& hitbox,
	float delta,
	uint substeps,
    entityid_t entity
) {
	float subDelta = delta / static_cast<float>(substeps);
	float linearDamping = hitbox.linearDamping * hitbox.friction;
	float gravityScale = hitbox.gravityScale;
	auto half = hitbox.getHalfSize();
    glm::vec3& pos = hitbox.position;
    glm::vec3& vel = hitbox.velocity;

	bool prevGrounded = hitbox.grounded;
	hitbox.grounded = false;
    // Разбиваем шаг на подшаги
    for (uint i = 0; i < substeps; ++i) {
		float prev_x = pos.x;
		float prev_y = pos.y;
		float prev_z = pos.z;

		if (hitbox.type == BodyType::Dynamic) {
			colisionCalc(
				chunks,
				hitbox,
				vel,
				pos,
				half,
				(prevGrounded && gravityScale > 0.0f) ? hitbox.stepHeight : 0.0f
			);
		}

		vel += gravity * subDelta * gravityScale;
		pos += vel * subDelta + gravity * gravityScale * subDelta * subDelta * 0.5f;
		if (hitbox.grounded && pos.y < prev_y) pos.y = prev_y;

		if (hitbox.crouching && hitbox.grounded) {
			AABB aabb;

			float y = pos.y - half.y - PhysicsSolver_Consts::EPS;
			hitbox.grounded = false;

			aabb = AABB(pos - half, pos + half);
            aabb.scale(glm::vec3(1.0f - PhysicsSolver_Consts::EPS * 4, 1.0f, 1.0f - PhysicsSolver_Consts::EPS * 4));
			for (int ix = 0; ix <= glm::ceil((half.x - PhysicsSolver_Consts::EPS) * 2); ++ix) {
                float x = (prev_x - half.x + PhysicsSolver_Consts::EPS) + ix;
                for (int iz = 0; iz <= glm::ceil((half.z - PhysicsSolver_Consts::EPS) * 2); ++iz) {
                    float z = (pos.z - half.z + PhysicsSolver_Consts::EPS) + iz;
                    if (chunks.isObstacleAt(x,y,z, aabb)){
						hitbox.grounded = true;
						break;
					}
				}
			}
			if (!hitbox.grounded) {
				pos.z = prev_z;
				vel.z = 0.0f;
			}

			hitbox.grounded = false;

			aabb = AABB(pos - half, pos + half);
            aabb.scale(glm::vec3(1.0f - PhysicsSolver_Consts::EPS * 4, 1.0f, 1.0f - PhysicsSolver_Consts::EPS * 4));
			for (int ix = 0; ix <= glm::ceil((half.x - PhysicsSolver_Consts::EPS) * 2); ++ix) {
                float x = (pos.x - half.x + PhysicsSolver_Consts::EPS) + ix;
                for (int iz = 0; iz <= glm::ceil((half.z - PhysicsSolver_Consts::EPS) * 2); ++iz) {
                    float z = (prev_z - half.z + PhysicsSolver_Consts::EPS) + iz;
                    if (chunks.isObstacleAt(x, y, z, aabb)){
						hitbox.grounded = true;
						break;
					}
				}
			}
			if (!hitbox.grounded) {
				pos.x = prev_x;
				vel.x = 0.0f;
			}
			hitbox.grounded = true;
		}
	}

	vel.x /= 1.0f + delta * linearDamping;
	vel.z /= 1.0f + delta * linearDamping;
	if (hitbox.verticalDamping > 0.0f) {
        vel.y /= 1.0f + delta * linearDamping * hitbox.verticalDamping;
	}

	AABB aabb;
    aabb.a = hitbox.position - hitbox.getHalfSize();
    aabb.b = hitbox.position + hitbox.getHalfSize();
    for (size_t i = 0; i < sensors.size(); ++i) {
        auto& sensor = *sensors[i];
        if (sensor.entity == entity) continue;
        bool sensored = false;
        switch (sensor.type) {
            case SensorType::AABB:
                sensored = aabb.intersects(sensor.calculated.aabb);
                break;
            case SensorType::RADIUS:
                sensored = glm::distance2(
                    hitbox.position, glm::vec3(sensor.calculated.radial)
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
	const GlobalChunks& chunks, 
    const glm::vec3& pos,
	const glm::vec3& half,
    float stepHeight
) {
	AABB aabb(-half, +half);
    aabb.scale(glm::vec3(1.0f - PhysicsSolver_Consts::EPS * 2, 1.0f, 1.0f - PhysicsSolver_Consts::EPS * 2));
    aabb = aabb + pos + glm::vec3(0.0f, stepHeight, 0.0f);
	if (stepHeight > 0.0f) {
        for (int ix = 0; ix <= glm::ceil((half.x - PhysicsSolver_Consts::EPS) * 2); ++ix) {
            float x = (pos.x - half.x) + ix;
            for (int iz = 0; iz <= glm::ceil((half.z - PhysicsSolver_Consts::EPS) * 2); ++iz) {
                float z = (pos.z - half.z) + iz;
                if (chunks.isObstacleAt(x, pos.y + half.y + stepHeight, z, aabb)) {
                    return 0.0f;
                }
            }
        }
    }
	return stepHeight;
}

template<int nx, int ny, int nz>
static bool calc_collision_neg(
    const GlobalChunks& chunks,
    glm::vec3& pos,
    glm::vec3& vel,
    const glm::vec3& half,
    float stepHeight
) {
    if (vel[nx] >= 0.0f) return false;

	glm::vec3 offset(0.0f, stepHeight + PhysicsSolver_Consts::EPS, 0.0f);
    for (int iy = 0; iy <= glm::ceil(((half - offset * 0.5f)[ny] - PhysicsSolver_Consts::EPS) * 2); ++iy) {
        glm::vec3 coord;
        coord[ny] = ((pos + offset)[ny] - half[ny] + PhysicsSolver_Consts::EPS) + iy;
        for (int iz = 0; iz <= glm::ceil((half[nz] - PhysicsSolver_Consts::EPS) * 2); ++iz){
            coord[nz] = (pos[nz] - half[nz] + PhysicsSolver_Consts::EPS) + iz;
            coord[nx] = (pos[nx] - half[nx] - PhysicsSolver_Consts::EPS);

            auto boxAABB = AABB(pos - half, pos + half);
            glm::vec3 scale(1.0f);
            scale[nz] = 1.0f - PhysicsSolver_Consts::EPS * 2.0f;
            boxAABB.scale(scale);
			boxAABB = boxAABB + offset;
			boxAABB.b.y -= stepHeight;

            if (const auto aabb = chunks.isObstacleAt(coord.x, coord.y, coord.z, boxAABB)) {
                vel[nx] = 0.0f;
                float newx = std::floor(coord[nx]) + half[nx] + aabb->max()[nx] + PhysicsSolver_Consts::EPS;
                if (newx - pos[nx] <= PhysicsSolver_Consts::EPS) {
                    pos[nx] = newx;
				}
				return true;
			}
		}
	}
	return false;
}

static bool calc_collision_neg_y(
    const GlobalChunks& chunks,
    glm::vec3& pos,
    glm::vec3& vel,
    const glm::vec3& half
) {
    if (vel.y >= 0.0f) {
        return false;
    }

    for (int ix = 0; ix <= glm::ceil((half.x - PhysicsSolver_Consts::EPS) * 2); ++ix) {
        glm::vec3 coord;
        coord.x = (pos.x - half.x + PhysicsSolver_Consts::EPS) + ix;
        for (int iz = 0; iz <= glm::ceil((half.z - PhysicsSolver_Consts::EPS) * 2); ++iz) {
            coord.z = (pos.z - half.z + PhysicsSolver_Consts::EPS) + iz;
            coord.y = (pos.y - half.y - PhysicsSolver_Consts::EPS);

            auto boxAABB = AABB(pos - half, pos + half);
            glm::vec3 scale(1.0f);
            scale.x = 1.0f - PhysicsSolver_Consts::EPS * 4.0f;
            scale.z = 1.0f - PhysicsSolver_Consts::EPS * 4.0f;
            boxAABB.scale(scale);

            if (const auto aabb = chunks.isObstacleAt(coord.x, coord.y, coord.z, boxAABB)) {
                vel.y = 0.0f;
                float newx = std::floor(coord.y) + half.y + aabb->max().y;
                if (newx - pos.y <= PhysicsSolver_Consts::EPS) {
                    pos.y = newx;
                }
                return true;
            }
        }
    }
    return false;
}

template <int nx, int ny, int nz>
static void calc_collision_pos(
    const GlobalChunks& chunks,
    glm::vec3& pos,
    glm::vec3& vel,
    const glm::vec3& half,
    float stepHeight
) {
    if (vel[nx] <= 0.0f) return;

    glm::vec3 offset(0.0f, stepHeight + PhysicsSolver_Consts::EPS * 2, 0.0f);
    for (int iy = 0; iy <= glm::ceil(((half - offset * 0.5f)[ny] - PhysicsSolver_Consts::EPS) * 2); ++iy) {
        glm::vec3 coord;
        coord[ny] = ((pos + offset)[ny] - half[ny] + PhysicsSolver_Consts::EPS) + iy;
        for (int iz = 0; iz <= glm::ceil((half[nz] - PhysicsSolver_Consts::EPS) * 2); ++iz) {
            coord[nz] = (pos[nz] - half[nz] + PhysicsSolver_Consts::EPS) + iz;
            coord[nx] = (pos[nx] + half[nx] + PhysicsSolver_Consts::EPS);

            auto boxAABB = AABB(pos - half, pos + half);
            glm::vec3 scale(1.0f);
            scale[nz] = 1.0f - PhysicsSolver_Consts::EPS * 2.0f;
            boxAABB.scale(scale);
			boxAABB = boxAABB + offset;
			boxAABB.b.y -= stepHeight;

            if (const auto aabb = chunks.isObstacleAt(coord.x, coord.y, coord.z, boxAABB)) {
                vel[nx] = 0.0f;
                float newx = std::floor(coord[nx]) - half[nx] + aabb->min()[nx] - PhysicsSolver_Consts::EPS;
                if (newx - pos[nx] >= -PhysicsSolver_Consts::EPS) {
                    pos[nx] = newx;
				}
				return;
			}
		}
	}
}

void PhysicsSolver::colisionCalc(
    const GlobalChunks& chunks,
    Hitbox& hitbox,
    glm::vec3& vel,
    glm::vec3& pos,
    const glm::vec3 half,
    float stepHeight
) {
    stepHeight = calc_step_height(chunks, pos, half, stepHeight);

    const AABB* aabb;

    calc_collision_neg<0, 1, 2>(chunks, pos, vel, half, stepHeight);
    calc_collision_pos<0, 1, 2>(chunks, pos, vel, half, stepHeight);

    calc_collision_neg<2, 1, 0>(chunks, pos, vel, half, stepHeight);
    calc_collision_pos<2, 1, 0>(chunks, pos, vel, half, stepHeight);

    if (calc_collision_neg_y(chunks, pos, vel, half)) {
        hitbox.grounded = true;
	}

	if (stepHeight > 0.0 && vel.y <= 0.0f) {
		AABB boxAABB = AABB(-half, +half);
        boxAABB.scale(glm::vec3(1.0f - PhysicsSolver_Consts::EPS * 2, 1.0f, 1.0f - PhysicsSolver_Consts::EPS * 2));
        boxAABB = boxAABB.translated(pos);

		for (int ix = 0; ix <= glm::ceil((half.x - PhysicsSolver_Consts::EPS) * 2); ++ix) {
            float x = (pos.x - half.x) + ix;
            for (int iz = 0; iz <= glm::ceil((half.z - PhysicsSolver_Consts::EPS) * 2); ++iz) {
                float z = (pos.z - half.z) + iz;
				float y = pos.y - half.y + PhysicsSolver_Consts::EPS;
                if ((aabb = chunks.isObstacleAt(x, y, z, boxAABB))) {
					vel.y = 0.0f;
					float newy = std::floor(y) + aabb->max().y + half.y;
					if (std::abs(newy - pos.y) <= PhysicsSolver_Consts::MAX_FIX + stepHeight) pos.y = newy;
					break;
				}
			}
		}
	}

	if (vel.y > 0.0f) {
		AABB boxAABB = AABB(-half, +half);
        boxAABB.scale(glm::vec3(1.0f - PhysicsSolver_Consts::EPS * 2, 1.0f, 1.0f - PhysicsSolver_Consts::EPS * 2));
        boxAABB = boxAABB.translated(pos);
		for (int ix = 0; ix <= glm::ceil((half.x - PhysicsSolver_Consts::EPS) * 2); ++ix) {
            float x = (pos.x - half.x + PhysicsSolver_Consts::EPS) + ix;
            for (int iz = 0; iz <= glm::ceil((half.z - PhysicsSolver_Consts::EPS) * 2); ++iz) {
                float z = (pos.z - half.z + PhysicsSolver_Consts::EPS) + iz;
				float y = pos.y + half.y + PhysicsSolver_Consts::EPS;
                if ((aabb = chunks.isObstacleAt(x, y, z, boxAABB))) {
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
	auto half = hitbox->getHalfSize();
	return x >= floor(pos.x - half.x) && x <= floor(pos.x + half.x) &&
			z >= floor(pos.z - half.z) && z <= floor(pos.z + half.z) &&
			y >= floor(pos.y - half.y) && y <= floor(pos.y + half.y);
}

bool PhysicsSolver::isBlockInside(int x, int y, int z, Block* def, blockstate state, Hitbox* hitbox) {
	const glm::vec3& pos = hitbox->position;
	auto half = hitbox->getHalfSize();
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
