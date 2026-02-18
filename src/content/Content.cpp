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
    DrawGroups* groups = new DrawGroups;
    for (const std::string& name : blockIds) {
        Block* def = blockDefs[name];
        def->rt.id = blockDefsIndices.size();
        def->rt.emissive = *((uint32_t*)def->emission);

        def->rt.solid = def->model == BlockModel::Cube;
        if (def->rotatable) {
            const AABB& hitbox = def->hitbox;
            for (uint i = 0; i < BlockRotProfile::MAX_COUNT; ++i) {
                AABB aabb = hitbox;
                def->rotations.variants[i].transform(aabb);
                def->rt.hitboxes[i] = aabb;
            }
        }

        blockDefsIndices.push_back(def);
        if (groups->find(def->drawGroup) == groups->end()) groups->insert(def->drawGroup);
    }
    ContentIndices* indices = new ContentIndices(blockDefsIndices);
    return new Content(indices, groups, blockDefs);
}

ContentIndices::ContentIndices(std::vector<Block*> blockDefs) : blockDefs(blockDefs) {
}

Content::Content(ContentIndices* indices, DrawGroups* drawGroups, std::unordered_map<std::string, Block*> blockDefs) : blockDefs(blockDefs), indices(indices), drawGroups(drawGroups) {
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
