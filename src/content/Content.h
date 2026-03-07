#ifndef CONTENT_CONTENT_H_
#define CONTENT_CONTENT_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <stdexcept>
#include <memory>

#include "../typedefs.h"

typedef std::set<ubyte> DrawGroups;

class Block;
class Content;
class Item;

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
    ContentType type;
public:
    namereuse_error(const std::string& msg, ContentType type) : std::runtime_error(msg), type(type) {}

    inline ContentType getType() const {
        return type;
    }
};

class ContentBuilder {
    std::unordered_map<std::string, Block*> blockDefs;
    std::vector<std::string> blockIds;

    std::unordered_map<std::string, Item*> itemDefs;
    std::vector<std::string> itemIds;
public:
    ~ContentBuilder();

    void add(Block* def);
    void add(Item* def);

    Block* createBlock(std::string id);
    Item* createItem(std::string id);

    void checkIdentifier(std::string id);
    ContentType checkContentType(std::string id);

    Content* build();
};

class ContentIndices {
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
    std::unordered_map<std::string, Block*> blockDefs;
    std::unordered_map<std::string, Item*> itemDefs;

    std::unique_ptr<ContentIndices> indices;
public:
    DrawGroups* const drawGroups;

    Content(ContentIndices* indices, DrawGroups* drawGroups, std::unordered_map<std::string, Block*> blockDefs, std::unordered_map<std::string, Item*> itemDefs);
    ~Content();

    inline ContentIndices* getIndices() const {
        return indices.get();
    }

    Block* findBlock(std::string id) const;
    Block* requireBlock(std::string id) const;

    Item* findItem(std::string id) const;
    Item* requireItem(std::string id) const;
};

#endif // CONTENT_CONTENT_H_
