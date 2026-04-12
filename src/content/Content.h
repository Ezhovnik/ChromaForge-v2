#ifndef CONTENT_CONTENT_H_
#define CONTENT_CONTENT_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <stdexcept>
#include <memory>

#include "../typedefs.h"

using DrawGroups = std::set<ubyte>;
template<class K, class V>
using UptrsMap = std::unordered_map<K, std::unique_ptr<V>>;

class Content;
struct Item;
struct Entity;
class ContentPackRuntime;
class Block;
struct BlockMaterial;
namespace rigging {
    class RigConfig;
}

enum class ContentType {
    None,
    Block,
    Item,
    Entity
};

inline const char* contenttype_name(ContentType type) {
    switch (type) {
        case ContentType::None: return "none";
        case ContentType::Block: return "block";
        case ContentType::Item: return "item";
        case ContentType::Entity: return "entity";
        default:
            return "unknown";
    }
}

class namereuse_error: public std::runtime_error {
private:
    ContentType type;
public:
    namereuse_error(const std::string& msg, ContentType type) : std::runtime_error(msg), type(type) {}

    inline ContentType getType() const {
        return type;
    }
};

template<class T>
class ContentUnitIndices {
private:
    std::vector<T*> defs;
public:
    ContentUnitIndices(std::vector<T*> defs) : defs(std::move(defs)) {}

    inline T* get(blockid_t id) const {
        if (id >= defs.size()) {
            return nullptr;
        }
        return defs[id];
    }

    inline size_t count() const {
        return defs.size();
    }

    inline const T* const* getDefs() const {
        return defs.data();
    }
};

class ContentIndices {
public:
    ContentUnitIndices<Block> blocks;
    ContentUnitIndices<Item> items;
    ContentUnitIndices<Entity> entities;

    ContentIndices(
        ContentUnitIndices<Block> blocks,
        ContentUnitIndices<Item> items,
        ContentUnitIndices<Entity> entities
    );
};

template<class T>
class ContentUnitDefs {
private:
    UptrsMap<std::string, T> defs;
public:
    ContentUnitDefs(std::unordered_map<std::string, std::unique_ptr<T>> defs) : defs(std::move(defs)) {}

    T* find(const std::string& id) const {
        const auto& found = defs.find(id);
        if (found == defs.end()) {
            return nullptr;
        }
        return found->second.get();
    }

    T& require(const std::string& id) const {
        const auto& found = defs.find(id);
        if (found == defs.end()) {
            throw std::runtime_error("Missing content unit " + id);
        }
        return *found->second;
    }
};

class Content {
private:
    std::unique_ptr<ContentIndices> indices;
    UptrsMap<std::string, ContentPackRuntime> packs;
    UptrsMap<std::string, BlockMaterial> blockMaterials;
    UptrsMap<std::string, rigging::RigConfig> rigs;
public:
    ContentUnitDefs<Block> blocks;
    ContentUnitDefs<Item> items;
    ContentUnitDefs<Entity> entities;
    std::unique_ptr<DrawGroups> const drawGroups;

    Content(
        std::unique_ptr<ContentIndices> indices, 
        std::unique_ptr<DrawGroups> drawGroups,
        ContentUnitDefs<Block> blocks,
        ContentUnitDefs<Item> items,
        ContentUnitDefs<Entity> entities,
        UptrsMap<std::string, ContentPackRuntime> packs,
        UptrsMap<std::string, BlockMaterial> blockMaterials,
        UptrsMap<std::string, rigging::RigConfig> rigs
    );
    ~Content();

    inline ContentIndices* getIndices() const {
        return indices.get();
    }

    const rigging::RigConfig* getRig(const std::string& id) const;
    const BlockMaterial* findBlockMaterial(const std::string& id) const;
    const ContentPackRuntime* getPackRuntime(const std::string& id) const;

    const UptrsMap<std::string, BlockMaterial>& getBlockMaterials() const;
    const UptrsMap<std::string, ContentPackRuntime>& getPacks() const;
};

#endif // CONTENT_CONTENT_H_
