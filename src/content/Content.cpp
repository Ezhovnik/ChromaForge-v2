#include "Content.h"

#include <stdexcept>

#include <glm/glm.hpp>

#include "../voxels/Block.h"
#include "../logger/Logger.h"

void ContentBuilder::add(Block* def) {
    if (blockDefs.find(def->name) != blockDefs.end()) {
        LOG_ERROR("Block name duplicate: {}", def->name);
        throw std::runtime_error("Block name duplicate: " + def->name);
    }
    blockDefs[def->name] = def;
    blockIds.push_back(def->name);
}

Content* ContentBuilder::build() {
    std::vector<Block*> blockDefsIndices;
    for (const std::string& name : blockIds) {
        Block* def = blockDefs[name];
        def->rt.id = blockDefsIndices.size();
        def->rt.emissive = *((uint32_t*)def->emission);

        const AABB& hitbox = def->hitbox;
        for (uint gy = 0; gy < BLOCK_AABB_GRID; ++gy) {
            for (uint gz = 0; gz < BLOCK_AABB_GRID; ++gz) {
                for (uint gx = 0; gx < BLOCK_AABB_GRID; ++gx) {
                    float x = gx / float(BLOCK_AABB_GRID);
                    float y = gy / float(BLOCK_AABB_GRID);
                    float z = gz / float(BLOCK_AABB_GRID);
                    bool flag = hitbox.inside({x, y, z});
                    if (!flag) def->rt.solid = false;
                    def->rt.hitboxGrid[gy][gz][gx] = flag;
                }
            }
        }

        blockDefsIndices.push_back(def);
    }
    ContentIndices* indices = new ContentIndices(blockDefsIndices);
    return new Content(indices, blockDefs);
}

ContentIndices::ContentIndices(std::vector<Block*> blockDefs) : blockDefs(blockDefs) {
}

Content::Content(ContentIndices* indices, std::unordered_map<std::string, Block*> blockDefs) : blockDefs(blockDefs), indices(indices) {
}

Content::~Content() {
    delete indices;
}

Block* Content::require(std::string id) const {
    auto found = blockDefs.find(id);
    if (found == blockDefs.end()) {
        LOG_ERROR("Missing block {}", id);
        throw std::runtime_error("Missing block " + id);
    }
    return found->second;
}
