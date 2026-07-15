#include <objects/Entt_Entity.h>

#include <entt/entt.hpp>

#include <objects/Transform.h>
#include <objects/Rigidbody.h>
#include <objects/ScriptComponents.h>
#include <objects/Entities.h>
#include <objects/Entity.h>
#include <objects/rigging.h>
#include <logic/scripting/scripting.h>

static inline std::string SAVED_DATA_VARNAME = "SAVED_DATA";

void Entt_Entity::setInterpolatedPosition(const glm::vec3& position) {
    getSkeleton().interpolation.refresh(position);
}

glm::vec3 Entt_Entity::getInterpolatedPosition() const {
    const auto& skeleton = getSkeleton();
    if (skeleton.interpolation.isEnabled()) {
        return skeleton.interpolation.getCurrent();
    }
    return getTransform().pos;
}

void Entt_Entity::destroy() {
    if (isValid()) {
        entities.despawn(id);
    }
}

rigging::Skeleton& Entt_Entity::getSkeleton() const {
    return registry.get<rigging::Skeleton>(entity);
}

void Entt_Entity::setRig(const rigging::SkeletonConfig* rigConfig) {
    auto& skeleton = registry.get<rigging::Skeleton>(entity);
    skeleton.config = rigConfig;
    skeleton.pose.matrices.resize(
        rigConfig->getBones().size(), glm::mat4(1.0f)
    );
    skeleton.calculated.matrices.resize(
        rigConfig->getBones().size(), glm::mat4(1.0f)
    );
}

dv::value Entt_Entity::serialize() const {
    const auto& eid = getID();
    const auto& def = eid.def;
    const auto& transform = getTransform();
    const auto& rigidbody = getRigidbody();
    const auto& skeleton = getSkeleton();
    const auto& scripts = getScripting();

    auto root = dv::object();
    root["def"] = def.name;
    root["uid"] = eid.uid;

    root[COMP_TRANSFORM] = transform.serialize();
    root[COMP_RIGIDBODY] =
        rigidbody.serialize(def.save.body.velocity, def.save.body.settings);

    if (skeleton.config->getName() != def.skeletonName) {
        root["skeleton"] = skeleton.config->getName();
    }
    if (def.save.skeleton.pose || def.save.skeleton.textures) {
        root[COMP_SKELETON] = skeleton.serialize(
            def.save.skeleton.pose, def.save.skeleton.textures
        );
    }
    if (!scripts.components.empty()) {
        auto& compsMap = root.object("comps");
        for (auto& comp : scripts.components) {
            auto data =
                scripting::get_component_value(comp->env, SAVED_DATA_VARNAME);
            compsMap[comp->name] = data;
        }
    }
    return root;
}

EntityId& Entt_Entity::getID() const {
    return registry.get<EntityId>(entity);
}

bool Entt_Entity::isValid() const {
    return registry.valid(entity);
}

Transform& Entt_Entity::getTransform() const {
    return registry.get<Transform>(entity);
}

ScriptComponents& Entt_Entity::getScripting() const {
    return registry.get<ScriptComponents>(entity);
}

const Entity& Entt_Entity::getDef() const {
    return registry.get<EntityId>(entity).def;
}

Rigidbody& Entt_Entity::getRigidbody() const {
    return registry.get<Rigidbody>(entity);
}

entityid_t Entt_Entity::getUID() const {
    return registry.get<EntityId>(entity).uid;
}

int64_t Entt_Entity::getPlayer() const {
    return registry.get<EntityId>(entity).player;
}

void Entt_Entity::setPlayer(int64_t id) {
    registry.get<EntityId>(entity).player = id;
}
