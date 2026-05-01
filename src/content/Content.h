#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <stdexcept>
#include <memory>
#include <optional>

#include <content/content_fwd.h>
#include <data/dv.h>

using DrawGroups = std::set<ubyte>;
template<class K, class V>
using UptrsMap = std::unordered_map<K, std::unique_ptr<V>>;

struct Item;
struct Entity;
class Block;
struct BlockMaterial;
struct Generator;
namespace rigging {
    class SkeletonConfig;
}

constexpr const char* contenttype_name(ContentType type) {
    switch (type) {
        case ContentType::None: return "none";
        case ContentType::Block: return "block";
        case ContentType::Item: return "item";
        case ContentType::Entity: return "entity";
        case ContentType::Generator: return "generator";
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

    inline const T* get(blockid_t id) const {
        if (id >= defs.size()) {
            return nullptr;
        }
        return defs[id];
    }

    inline const T& require(blockid_t id) const {
        return *defs.at(id);
    }

    inline size_t count() const {
        return defs.size();
    }

    inline const auto& getIterable() const {
        return defs;
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

    const T* find(const std::string& id) const {
        const auto& found = defs.find(id);
        if (found == defs.end()) {
            return nullptr;
        }
        return found->second.get();
    }

    const T& require(const std::string& id) const {
        const auto& found = defs.find(id);
        if (found == defs.end()) {
            throw std::runtime_error("Missing content unit " + id);
        }
        return *found->second;
    }

    const auto& getDefs() const {
        return defs;
    }
};

class ResourceIndices {
private:
    std::vector<std::string> names;
    std::unordered_map<std::string, size_t> indices;
    std::unique_ptr<std::vector<dv::value>> savedData;
public:
    ResourceIndices() : savedData(std::make_unique<std::vector<dv::value>>()){
    }

    static constexpr size_t MISSING = SIZE_MAX;

    void add(std::string name, dv::value map) {
        indices[name] = names.size();
        names.push_back(name);
        savedData->push_back(std::move(map));
    }

    const std::string& getName(size_t index) const {
        return names.at(index);
    }

    size_t indexOf(const std::string& name) const {
        const auto& found = indices.find(name);
        if (found != indices.end()) {
            return found->second;
        }
        return MISSING;
    }

    const dv::value& getSavedData(size_t index) const {
        return savedData->at(index);
    }

    void saveData(size_t index, dv::value map) const {
        savedData->at(index) = std::move(map);
    }

    size_t size() const {
        return names.size();
    }
};

constexpr const char* to_string(ResourceType type) {
    switch (type) {
        case ResourceType::Camera: return "camera";
        default: return "unknown";
    }
}

inline std::optional<ResourceType> ResourceType_from(std::string_view str) {
    if (str == "camera") {
        return ResourceType::Camera;
    }
    return std::nullopt;
}

using ResourceIndicesSet = ResourceIndices[RESOURCE_TYPES_COUNT];

class Content {
private:
    std::unique_ptr<ContentIndices> indices;
    UptrsMap<std::string, ContentPackRuntime> packs;
    UptrsMap<std::string, BlockMaterial> blockMaterials;
    UptrsMap<std::string, rigging::SkeletonConfig> skeletons;
public:
    ContentUnitDefs<Block> blocks;
    ContentUnitDefs<Item> items;
    ContentUnitDefs<Entity> entities;
    ContentUnitDefs<Generator> generators;
    std::unique_ptr<DrawGroups> const drawGroups;
    ResourceIndicesSet resourceIndices {};

    Content(
        std::unique_ptr<ContentIndices> indices, 
        std::unique_ptr<DrawGroups> drawGroups,
        ContentUnitDefs<Block> blocks,
        ContentUnitDefs<Item> items,
        ContentUnitDefs<Entity> entities,
        ContentUnitDefs<Generator> generators,
        UptrsMap<std::string, ContentPackRuntime> packs,
        UptrsMap<std::string, BlockMaterial> blockMaterials,
        UptrsMap<std::string, rigging::SkeletonConfig> skeletons,
        ResourceIndicesSet resourceIndices
    );
    ~Content();

    inline ContentIndices* getIndices() const {
        return indices.get();
    }

    inline const ResourceIndices& getIndices(ResourceType type) const {
        return resourceIndices[static_cast<size_t>(type)];
    }

    const rigging::SkeletonConfig* getSkeleton(const std::string& id) const;
    const BlockMaterial* findBlockMaterial(const std::string& id) const;
    const ContentPackRuntime* getPackRuntime(const std::string& id) const;

    const UptrsMap<std::string, BlockMaterial>& getBlockMaterials() const;
    const UptrsMap<std::string, ContentPackRuntime>& getPacks() const;
    const UptrsMap<std::string, rigging::SkeletonConfig>& getSkeletons() const;
};
