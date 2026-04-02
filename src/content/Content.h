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

class Content;
class Item;
class ContentPackRuntime;
class Block;
struct BlockMaterial;

enum class ContentType {
    None,
    Block,
    Item
};

inline const char* contenttype_name(ContentType type) {
    switch (type) {
        case ContentType::None: return "none";
        case ContentType::Block: return "block";
        case ContentType::Item: return "item";
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

class ContentIndices {
private:
    std::vector<Block*> blockDefs;
    std::vector<Item*> itemDefs;
public:
    ContentIndices(std::vector<Block*> blockDefs, std::vector<Item*> itemDefs);

    inline Block* getBlockDef(blockid_t id) const {
        if (id >= blockDefs.size()) return nullptr;
        return blockDefs[id];
    }

    inline Item* getItemDef(itemid_t id) const {
        if (id >= itemDefs.size()) return nullptr;
        return itemDefs[id];
    }

    inline size_t countBlockDefs() const {
        return blockDefs.size();
    }

    inline size_t countItemDefs() const {
        return itemDefs.size();
    }

    const Block* const* getBlockDefs() const {
        return blockDefs.data();
    }

    const Item* const* getItemDefs() const {
        return itemDefs.data();
    }
};

class Content {
private:
    std::unordered_map<std::string, std::unique_ptr<Block>> blockDefs;
    std::unordered_map<std::string, std::unique_ptr<Item>> itemDefs;

    std::unique_ptr<ContentIndices> indices;

    std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>> packs;

    std::unordered_map<std::string, std::unique_ptr<BlockMaterial>> blockMaterials;
public:
    std::unique_ptr<DrawGroups> const drawGroups;

    Content(
        std::unique_ptr<ContentIndices> indices,
        std::unique_ptr<DrawGroups> drawGroups, 
        std::unordered_map<std::string, std::unique_ptr<Block>> blockDefs,
        std::unordered_map<std::string, std::unique_ptr<Item>> itemDefs,
        std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>> packs,
        std::unordered_map<std::string, std::unique_ptr<BlockMaterial>> blockMaterials
    );
    ~Content();

    inline ContentIndices* getIndices() const {
        return indices.get();
    }

    Block* findBlock(const std::string& id) const;
    Block& requireBlock(const std::string& id) const;

    Item* findItem(const std::string& id) const;
    Item& requireItem(const std::string& id) const;

    const BlockMaterial* findBlockMaterial(const std::string& id) const;
    const std::unordered_map<std::string, std::unique_ptr<BlockMaterial>>& getBlockMaterials() const;

    const ContentPackRuntime* getPackRuntime(const std::string& id) const;
    const std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>>& getPacks() const;
};

#endif // CONTENT_CONTENT_H_
