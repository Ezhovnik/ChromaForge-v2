#ifndef CONTENT_CONTENT_BUILDER_H_
#define CONTENT_CONTENT_BUILDER_H_

#include <memory>
#include <vector>
#include <unordered_map>

#include "../items/Item.h"
#include "../voxels/Block.h"
#include "../objects/Entity.h"
#include "../content/Content.h"
#include "../content/ContentPack.h"

template<class T>
class ContentUnitBuilder {
private:
    std::unordered_map<std::string, ContentType>& allNames;
    ContentType type;

    void checkIdentifier(const std::string& id) {
        const auto& found = allNames.find(id);
        if (found != allNames.end()) {
            throw namereuse_error("Name " + id + " is already used", found->second);
        }
    }
public:
    UptrsMap<std::string, T> defs;
    std::vector<std::string> names;

    ContentUnitBuilder(
        std::unordered_map<std::string, ContentType>& allNames,
        ContentType type
    ) : allNames(allNames), type(type) {}

    T& create(const std::string& id) {
        auto found = defs.find(id);
        if (found != defs.end()) {
            return *found->second;
        }
        checkIdentifier(id);
        allNames[id] = type;
        names.push_back(id);
        defs[id] = std::make_unique<T>(id);
        return *defs[id];
    }

    auto build() {
        return std::move(defs);
    }
};

class ContentBuilder {
    UptrsMap<std::string, BlockMaterial> blockMaterials;
    UptrsMap<std::string, rigging::RigConfig> rigs;
    UptrsMap<std::string, ContentPackRuntime> packs;
    std::unordered_map<std::string, ContentType> allNames;
public:
    ContentUnitBuilder<Block> blocks {allNames, ContentType::Block};
    ContentUnitBuilder<Item> items {allNames, ContentType::Item};
    ContentUnitBuilder<Entity> entities {allNames, ContentType::Entity};

    ~ContentBuilder();

    void add(std::unique_ptr<ContentPackRuntime> pack);
    void add(std::unique_ptr<rigging::RigConfig> rig);

    BlockMaterial& createBlockMaterial(const std::string& id);

    std::unique_ptr<Content> build();
};

#endif // CONTENT_CONTENT_BUILDER_H_
