#ifndef OBJECTS_ENTITIES_HPP_
#define OBJECTS_ENTITIES_HPP_

#include <unordered_map>
#include <optional>

#include <glm/glm.hpp>
#include <entt/entity/registry.hpp>

#include "../typedefs.h"
#include "../physics/Hitbox.h"

struct EntityId {
    entityid_t uid;
};

struct Transform {
    glm::vec3 pos;
    glm::vec3 size;
    glm::mat3 rot;
    glm::mat4 combined;

    void refresh();
};

class Level;
class Assets;
class ModelBatch;
class Frustum;
class Rig;
class LineBatch;
struct Entity;

class Entt_Entity {
private:
    entt::registry& registry;
    const entt::entity entity;
public:
    Entt_Entity(entt::registry& registry, const entt::entity entity)
    : registry(registry), entity(entity) {}

    bool isValid() const {
        return registry.valid(entity);
    }

    Transform& getTransform() const {
        return registry.get<Transform>(entity);
    }

    Hitbox& getHitbox() const {
        return registry.get<Hitbox>(entity);
    }

    entityid_t getUID() const {
        return registry.get<EntityId>(entity).uid;
    }
};

class Entities {
private:
    entt::registry registry;
    Level* level;
    std::unordered_map<entityid_t, entt::entity> entities;
    entityid_t nextID = 1;
public:
    Entities(Level* level);
    void updatePhysics(float delta);
    void render(Assets* assets, ModelBatch& batch, Frustum& frustum);
    void renderDebug(LineBatch& batch);

    entityid_t spawn(Entity& def, glm::vec3 pos);

    std::optional<Entt_Entity> get(entityid_t id) {
        const auto& found = entities.find(id);
        if (found != entities.end()) {
            return Entt_Entity(registry, found->second);
        }
        return std::nullopt;
    }
};

#endif // OBJECTS_ENTITIES_H_
