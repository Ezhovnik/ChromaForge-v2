#ifndef CONTENT_CONTENT_H_
#define CONTENT_CONTENT_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <set>

#include "../typedefs.h"

typedef std::set<ubyte> DrawGroups;

class Block;
class Content;

class ContentBuilder {
    std::unordered_map<std::string, Block*> blockDefs;
    std::vector<std::string> blockIds;
public:
    void add(Block* def);

    Content* build();
};

class ContentIndices {
    std::vector<Block*> blockDefs;
public:
    ContentIndices(std::vector<Block*> blockDefs);

    inline Block* getBlockDef(blockid_t id) const {
        if (id >= blockDefs.size()) return nullptr;
        return blockDefs[id];
    }

    inline size_t countBlockDefs() const {
        return blockDefs.size();
    }

    const Block* const* getBlockDefs() const {
        return blockDefs.data();
    }
};

class Content {
    std::unordered_map<std::string, Block*> blockDefs;
public:
    ContentIndices* const indices;
    DrawGroups* const drawGroups;
    
    Content(ContentIndices* indices, DrawGroups* drawGroups, std::unordered_map<std::string, Block*> blockDefs);
    ~Content();
    
    Block* require(std::string id) const;
};

#endif // CONTENT_CONTENT_H_
