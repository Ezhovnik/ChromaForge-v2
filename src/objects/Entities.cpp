#include "Entities.h"

#include <glm/ext/matrix_transform.hpp>

#include "../assets/Assets.h"
#include "../world/Level.h"
#include "../physics/Hitbox.h"
#include "../physics/PhysicsSolver.h"
#include "../graphics/render/ModelBatch.h"
#include "../graphics/core/Model.h"
#include "../math/FrustumCulling.h"
#include "../graphics/core/LineBatch.h"
#include "../objects/Entity.h"
#include "../logic/scripting/scripting.h"

void Transform::refresh() {
    combined = glm::mat4(1.0f);
    combined = glm::translate(combined, pos);
    combined = glm::scale(combined, size);
    combined = combined * glm::mat4(rot);
}

void Entt_Entity::destroy() {
    if (isValid()){
        entities.despawn(id);
    }
}

Entities::Entities(Level* level) : level(level) {
}

entityid_t Entities::spawn(Entity& def, glm::vec3 pos) {
    auto entity = registry.create();
    glm::vec3 size(1);
    auto id = nextID++;
    registry.emplace<EntityId>(entity, static_cast<entityid_t>(id), def);
    registry.emplace<Transform>(entity, pos, size / 4.0f, glm::mat3(1.0f));
    registry.emplace<Rigidbody>(entity, true, Hitbox {pos, def.hitbox});
    auto& scripting = registry.emplace<Scripting>(entity, entity_funcs_set {}, nullptr);

    entities[id] = entity;
    scripting.env = scripting::on_entity_spawn(def, id, scripting.funcsset);
    return id;
}

void Entities::despawn(entityid_t id) {
    if (auto entity = get(id)) {
        scripting::on_entity_despawn(entity->getDef(), *entity);
        registry.destroy(get(id)->getHandler());
    }
}

void Entities::update() {
    scripting::on_entities_update();
}

void Entities::renderDebug(LineBatch& batch) {
    batch.setLineWidth(1.0f);
    auto view = registry.view<Transform, Rigidbody>();
    for (auto [entity, transform, rigidbody] : view.each()) {
        auto& hitbox = rigidbody.hitbox;
        batch.box(hitbox.position, hitbox.halfsize * 2.0f, glm::vec4(1.0f));
    }
}

void Entities::updatePhysics(float delta){
    auto view = registry.view<EntityId, Transform, Rigidbody>();
    auto physics = level->physics.get();
    for (auto [entity, eid, transform, rigidbody] : view.each()) {
        if (!rigidbody.enabled) continue;
        auto& hitbox = rigidbody.hitbox;
        bool grounded = hitbox.grounded;
        physics->step(
            level->chunks.get(),
            &hitbox,
            delta,
            10,
            false,
            1.0f,
            true
        );
        hitbox.linearDamping = hitbox.grounded * 12;
        transform.pos = hitbox.position;
        // transform.rot = glm::rotate(glm::mat4(transform.rot), delta, glm::vec3(0, 1, 0));
        if (hitbox.grounded && !grounded) {
            scripting::on_entity_grounded(eid.def, *get(eid.uid));
        }
    }
}

void Entities::clean() {
    for (auto it = entities.begin(); it != entities.end();) {
        if (registry.valid(it->second)) {
            ++it;
        } else {
            it = entities.erase(it);
        }
    }
}

void Entities::render(Assets* assets, ModelBatch& batch, Frustum& frustum) {
    auto view = registry.view<Transform>();
    auto model = assets->get<model::Model>("cube");
    for (auto [entity, transform] : view.each()) {
        const auto& pos = transform.pos;
        const auto& size = transform.size;
        if (frustum.isBoxVisible(pos - size, pos + size)) {
            transform.refresh();
            batch.pushMatrix(transform.combined);
            batch.draw(model);
            batch.popMatrix();
        }
    }
}
