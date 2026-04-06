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

struct Rigidbody {
    bool enabled = true;
    Hitbox hitbox;
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
    entityid_t id;
    entt::registry& registry;
    const entt::entity entity;
public:
    Entt_Entity(entityid_t id, entt::registry& registry, const entt::entity entity)
    : id(id), registry(registry), entity(entity) {}

    entityid_t getID() const {
        return id;
    }

    bool isValid() const {
        return registry.valid(entity);
    }

    Transform& getTransform() const {
        return registry.get<Transform>(entity);
    }

    Rigidbody& getRigidbody() const {
        return registry.get<Rigidbody>(entity);
    }

    entityid_t getUID() const {
        return registry.get<EntityId>(entity).uid;
    }

    void destroy() {
        registry.destroy(entity);
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
    void clean();

    entityid_t spawn(Entity& def, glm::vec3 pos);

    std::optional<Entt_Entity> get(entityid_t id) {
        const auto& found = entities.find(id);
        if (found != entities.end() && registry.valid(found->second)) {
            return Entt_Entity(id, registry, found->second);
        }
        return std::nullopt;
    }

    inline size_t size() const {
        return entities.size();
    }
};

#endif // OBJECTS_ENTITIES_H_
