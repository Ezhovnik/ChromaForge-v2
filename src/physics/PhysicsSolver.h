#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <typedefs.h>
#include <physics/Hitbox.h>

class GlobalChunks;
class Block;
class Entities;
struct blockstate;

// Класс для решения физических взаимодействий объектов с воксельным миром.
class PhysicsSolver {
public:
    PhysicsSolver(glm::vec3 gravity); // Конструтор

    void step(
        const GlobalChunks& chunks,
        Hitbox& hitbox,
        float delta,
        uint substeps,
        entityid_t entity
    ); // Выполняет один шаг физического моделирования для указанного хитбокса.

    auto& getSensorsWriteable() {
        return sensors;
    }

    auto& getSolidHitboxesWriteable() {
        return solidHitboxes;
    }

    void removeSensor(Sensor* sensor);
private:
    glm::vec3 gravity;
    std::vector<Sensor*> sensors;
    std::vector<Hitbox*> solidHitboxes;

    void calcCollisions(
        const GlobalChunks& chunks,
        Hitbox& hitbox,
        glm::vec3& vel,
        glm::vec3& pos,
        const glm::vec3& half,
        float stepHeight
    );

    void calcSubstep(
        const GlobalChunks& chunks,
        Hitbox& hitbox,
        glm::vec3& vel,
        glm::vec3& pos,
        bool prevGrounded,
        float dt,
        int substeps
    );
};
