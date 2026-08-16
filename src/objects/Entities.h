#pragma once

#include <unordered_map>
#include <optional>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <entt/entity/fwd.hpp>

#include <constants.h>
#include <typedefs.h>
#include <physics/Hitbox.h>
#include <util/Clock.h>
#include <objects/Transform.h>
#include <objects/Rigidbody.h>
#include <objects/ScriptComponents.h>

struct Entity;
class Level;
class Assets;
class Entt_Entity;
class ModelBatch;
class Frustum;
class LineBatch;
class Entities;
namespace rigging {
    struct Skeleton;
    class SkeletonConfig;
}
class DrawContext;

class Entities final {
    std::unique_ptr<entt::registry> registry;
    Level& level;
    std::unordered_map<entityid_t, entt::entity> entities;
    std::unordered_map<entt::entity, entityid_t> uids;
    entityid_t nextID = 1;
    util::Clock sensorsSparkClock;
    util::Clock updateSparkClock;
    Assets* assets = nullptr;

    void updateSensors(
        Rigidbody& body, const Transform& tsf, std::vector<Sensor*>& sensors
    );
    void preparePhysics(float deltaTime);
public:
    Entities(Level& level);

    ~Entities();

    void setAssets(Assets& assets);

    void updatePhysics(float delta);
    void update(float deltaTime);
    void render(
        const Assets& assets,
        ModelBatch& batch,
        const Frustum* frustum,
        entityid_t fpsEntity
    );
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

    struct RaycastSettings {
        entityid_t ignoredUid;
        bool solidEntitiesOnly = false;
        bool entityFilterExcludeMode = false;
        const std::set<entitydefid_t>* entitiesFilter = nullptr;
    };

    struct RaycastResult {
        entityid_t entity;
        glm::ivec3 normal;
        float distance;
    };

    std::optional<RaycastResult> rayCast(
        glm::vec3 start,
        glm::vec3 dir,
        float maxDistance,
        const RaycastSettings& settings
    );

    void loadEntities(dv::value map);
    void loadEntity(const dv::value& map);
    void loadEntity(const dv::value& map, Entt_Entity entity);
    void onSave(const Entt_Entity& entity);

    bool hasBlockingInside(AABB aabb);
    std::vector<Entt_Entity> getAllInside(AABB aabb);
    std::vector<Entt_Entity> getAllInRadius(glm::vec3 center, float radius);

    dv::value serialize(const std::vector<Entt_Entity>& entities);

    void setNextID(entityid_t id) {
        nextID = id;
    }
    inline entityid_t peekNextID() const {
        return nextID;
    }

    std::optional<Entt_Entity> get(entityid_t id);

    inline size_t size() const {
        return entities.size();
    }
};
