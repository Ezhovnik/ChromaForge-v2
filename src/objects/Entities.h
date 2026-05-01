#pragma once

#include <unordered_map>
#include <optional>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <entt/entity/registry.hpp>

#include <typedefs.h>
#include <physics/Hitbox.h>
#include <data/dv.h>
#include <util/Clock.h>

struct entity_funcs_set {
    bool init;
    bool on_despawn;
    bool on_grounded;
    bool on_fall;
    bool on_sensor_enter;
    bool on_sensor_exit;
    bool on_save;
    bool on_aim_on;
    bool on_aim_off;
    bool on_attacked;
    bool on_used;
};

struct Entity;

struct EntityId {
    entityid_t uid;
    const Entity& def;
    bool destroyFlag = false;
};

struct Rigidbody {
    bool enabled = true;
    Hitbox hitbox;
    std::vector<Sensor> sensors;
};

struct Transform {
    static inline constexpr float EPS = 0.0000001f; 

    glm::vec3 pos;
    glm::vec3 size;
    glm::mat3 rot;
    glm::mat4 combined;
    bool dirty = true;

    glm::vec3 displayPos;
    glm::vec3 displaySize;

    void refresh();

    inline void setRot(glm::mat3 m) {
        rot = m;
        dirty = true;
    }

    inline void setSize(glm::vec3 v) {
        if (glm::distance2(displaySize, v) >= EPS) {
            dirty = true;
        }
        size = v;
    }

    inline void setPos(glm::vec3 v) {
        if (glm::distance2(displayPos, v) >= EPS) {
            dirty = true;
        }
        pos = v;
    }
};

struct UserComponent {
    std::string name;
    entity_funcs_set funcsset;
    scriptenv env;

    UserComponent(
        const std::string& name,
        entity_funcs_set funcsset,
        scriptenv env
    ) : name(name), funcsset(funcsset), env(env) {}
};

struct ScriptComponents {
    std::vector<std::unique_ptr<UserComponent>> components;

    ScriptComponents() = default;

    ScriptComponents(ScriptComponents&& other)
        : components(std::move(other.components)) {
    }
};

class Level;
class Assets;
class ModelBatch;
class Frustum;
class LineBatch;
class Entities;
namespace rigging {
    struct Skeleton;
    class SkeletonConfig;
}
class DrawContext;

class Entt_Entity {
private:
    Entities& entities;
    entityid_t id;
    entt::registry& registry;
    const entt::entity entity;
public:
    Entt_Entity(
        Entities& entities,
        entityid_t id, 
        entt::registry& registry, 
        const entt::entity entity
    ) : entities(entities),
        id(id),
        registry(registry),
        entity(entity) {}

    ScriptComponents& getScripting() const {
        return registry.get<ScriptComponents>(entity);
    }

    EntityId& getID() const {
        return registry.get<EntityId>(entity);
    }

    bool isValid() const {
        return registry.valid(entity);
    }

    const Entity& getDef() const {
        return registry.get<EntityId>(entity).def;
    }

    Transform& getTransform() const {
        return registry.get<Transform>(entity);
    }

    Rigidbody& getRigidbody() const {
        return registry.get<Rigidbody>(entity);
    }

    rigging::Skeleton& getSkeleton() const;

    void setRig(const rigging::SkeletonConfig* skeletonConfig);

    entityid_t getUID() const {
        return registry.get<EntityId>(entity).uid;
    }

    entt::entity getHandler() const {
        return entity;
    }

    void destroy();
};

class Entities {
private:
    entt::registry registry;
    Level* level;
    std::unordered_map<entityid_t, entt::entity> entities;
    std::unordered_map<entt::entity, entityid_t> uids;
    entityid_t nextID = 1;
    util::Clock sensorsSparkClock;
    util::Clock updateSparkClock;

    void updateSensors(
        Rigidbody& body, const Transform& tsf, std::vector<Sensor*>& sensors
    );
    void preparePhysics(float deltaTime);
public:
    struct RaycastResult {
        entityid_t entity;
        glm::ivec3 normal;
        float distance;
    };

    Entities(Level* level);

    void updatePhysics(float delta);
    void update(float deltaTime);
    void render(Assets* assets, ModelBatch& batch, const Frustum* frustum, float deltaTime, bool pause);
    void renderDebug(LineBatch& batch, const Frustum* frustum, const DrawContext& ctx);
    void clean();

    entityid_t spawn(
        const Entity& def,
        glm::vec3 position,
        dv::value args=nullptr,
        dv::value saved=nullptr,
        entityid_t uid=0
    );
    void despawn(entityid_t id);
    void despawn(std::vector<Entt_Entity> entities);

    std::optional<RaycastResult> rayCast(
        glm::vec3 start,
        glm::vec3 dir,
        float maxDistance,
        entityid_t ignore = -1
    );

    void loadEntities(dv::value map);
    void loadEntity(const dv::value& map);
    void loadEntity(const dv::value& map, Entt_Entity entity);
    void onSave(const Entt_Entity& entity);

    bool hasBlockingInside(AABB aabb);
    std::vector<Entt_Entity> getAllInside(AABB aabb);
    std::vector<Entt_Entity> getAllInRadius(glm::vec3 center, float radius);

    dv::value serialize(const Entt_Entity& entity);
    dv::value serialize(const std::vector<Entt_Entity>& entities);

    void setNextID(entityid_t id) {
        nextID = id;
    }
    inline entityid_t peekNextID() const {
        return nextID;
    }

    std::optional<Entt_Entity> get(entityid_t id) {
        const auto& found = entities.find(id);
        if (found != entities.end() && registry.valid(found->second)) {
            return Entt_Entity(*this, id, registry, found->second);
        }
        return std::nullopt;
    }

    inline size_t size() const {
        return entities.size();
    }
};
