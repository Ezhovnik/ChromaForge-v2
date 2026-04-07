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
#include "../objects/rigging.h"
#include "../debug/Logger.h"

void Transform::refresh() {
    combined = glm::mat4(1.0f);
    combined = glm::translate(combined, pos);
    combined = glm::scale(combined, size);
    combined = combined * glm::mat4(rot);
    dirty = false;
}

void Entt_Entity::destroy() {
    if (isValid()){
        entities.despawn(id);
    }
}

rigging::Rig& Entt_Entity::getModeltree() const {
    return registry.get<rigging::Rig>(entity);
}

Entities::Entities(Level* level) : level(level) {
}

entityid_t Entities::spawn(
    Assets* assets,
    Entity& def,
    glm::vec3 pos,
    dynamic::Value args
) {
    auto rig = assets->get<rigging::RigConfig>(def.rigName);
    if (rig == nullptr) {
        LOG_ERROR("Rig {} not found", def.rigName);
        throw std::runtime_error("Rig " + def.rigName + " not found");
    }
    auto entity = registry.create();
    glm::vec3 size(1);
    auto id = nextID++;
    registry.emplace<EntityId>(entity, static_cast<entityid_t>(id), def);
    registry.emplace<Transform>(entity, pos, size, glm::mat3(1.0f));
    auto& body = registry.emplace<Rigidbody>(
        entity, true, Hitbox {pos, def.hitbox}, std::vector<Trigger>{}
    );
    for (auto& box : def.boxTriggers) {
        body.triggers.emplace_back(Trigger{
            true,
            id,
            box,
            AABB{},
            {},
            {},
            [=](auto entityid, auto index, auto otherid) {
                if (auto entity = get(entityid)) {
                    if (entity->isValid()) {
                        scripting::on_trigger_enter(*entity, index, otherid);
                    }
                }
            },
            [=](auto entityid, auto index, auto otherid) {
                if (auto entity = get(entityid)) {
                    if (entity->isValid()) {
                        scripting::on_trigger_exit(*entity, index, otherid);
                    }
                }
            }}
        );
    }
    auto& scripting = registry.emplace<Scripting>(
        entity, entity_funcs_set {}, nullptr
    );

    entities[id] = entity;
    registry.emplace<rigging::Rig>(entity, rig->instance());
    scripting.env = scripting::on_entity_spawn(
        def, id, scripting.funcsset, std::move(args)
    );
    return id;
}

void Entities::despawn(entityid_t id) {
    if (auto entity = get(id)) {
        auto& eid = entity->getID();
        if (!eid.destroyFlag) {
            eid.destroyFlag = true;
            scripting::on_entity_despawn(entity->getDef(), *entity);
        }
    }
}

void Entities::update() {
    scripting::on_entities_update();
    auto view = registry.view<Transform>();
    for (auto [entity, transform] : view.each()) {
        if (transform.dirty) transform.refresh();
    }
}

void Entities::renderDebug(LineBatch& batch, const Frustum& frustum) {
    batch.setLineWidth(1.0f);
    auto view = registry.view<Transform, Rigidbody>();
    for (auto [entity, transform, rigidbody] : view.each()) {
        auto& hitbox = rigidbody.hitbox;
        const auto& pos = transform.pos;
        const auto& size = transform.size;
        if (!frustum.isBoxVisible(pos - size, pos + size)) {
            continue;
        }
        batch.box(hitbox.position, hitbox.halfsize * 2.0f, glm::vec4(1.0f));

        for (auto& trigger : rigidbody.triggers) {
            batch.box(
                trigger.calculated.center(), 
                trigger.calculated.size(), 
                glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)
            );
        }
    }
}

void Entities::preparePhysics() {
    static uint64_t frameid = 0;
    ++frameid;
    auto view = registry.view<EntityId, Transform, Rigidbody>();
    auto physics = level->physics.get();
    std::vector<Trigger*> triggers;
    for (auto [entity, eid, transform, rigidbody] : view.each()) {
        if (!rigidbody.enabled) continue;
        if ((eid.uid + frameid) % 3 != 0) continue;
        for (size_t i = 0; i < rigidbody.triggers.size(); ++i) {
            auto& trigger = rigidbody.triggers[i];
            for (auto oid : trigger.prevEntered) {
                if (trigger.nextEntered.find(oid) == trigger.nextEntered.end()) {
                    trigger.exitCallback(trigger.entity, i, oid);
                }
            }
            trigger.prevEntered = trigger.nextEntered;
            trigger.nextEntered.clear();
            trigger.calculated = trigger.aabb;
            trigger.calculated.transform(transform.combined);
            triggers.push_back(&trigger);
        }
    }
    physics->setTriggers(std::move(triggers));
}

void Entities::updatePhysics(float delta) {
    preparePhysics();

    auto view = registry.view<EntityId, Transform, Rigidbody>();
    auto physics = level->physics.get();
    for (auto [entity, eid, transform, rigidbody] : view.each()) {
        if (!rigidbody.enabled) continue;
        auto& hitbox = rigidbody.hitbox;
        auto prevVel = hitbox.velocity;
        bool grounded = hitbox.grounded;

        float vel = glm::length(prevVel);
        int substeps = static_cast<int>(delta * vel * 20);
        substeps = std::min(100, std::max(2, substeps));
        physics->step(
            level->chunks.get(),
            &hitbox,
            delta,
            substeps,
            false,
            1.0f,
            true,
            eid.uid
        );
        hitbox.linearDamping = hitbox.grounded * 24;
        transform.setPos(hitbox.position);
        if (hitbox.grounded && !grounded) {
            scripting::on_entity_grounded(
                *get(eid.uid),
                glm::length(prevVel - hitbox.velocity)
            );
        }
        if (!hitbox.grounded && grounded) {
            scripting::on_entity_fall(*get(eid.uid));
        }
    }
}

void Entities::clean() {
    for (auto it = entities.begin(); it != entities.end();) {
        if (!registry.get<EntityId>(it->second).destroyFlag) {
            ++it;
        } else {
            registry.destroy(it->second);
            it = entities.erase(it);
        }
    }
}

void Entities::render(Assets* assets, ModelBatch& batch, const Frustum& frustum) {
    auto view = registry.view<Transform, rigging::Rig>();
    for (auto [entity, transform, rig] : view.each()) {
        const auto& pos = transform.pos;
        const auto& size = transform.size;
        if (frustum.isBoxVisible(pos - size, pos + size)) {
            const auto* rigConfig = rig.config;
            rigConfig->render(assets, batch, rig, transform.combined);
        }
    }
}
