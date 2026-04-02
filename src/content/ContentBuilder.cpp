#include "ContentBuilder.h"

#include "../debug/Logger.h"

ContentBuilder::~ContentBuilder() {}

void ContentBuilder::add(std::unique_ptr<ContentPackRuntime> pack) {
    packs[pack->getId()] = std::move(pack);
}

Block& ContentBuilder::createBlock(const std::string& id) {
    auto found = blockDefs.find(id);
    if (found != blockDefs.end()) {
        return *found->second;
    }
    checkIdentifier(id);
    blockIds.push_back(id);
    blockDefs[id] = std::make_unique<Block>(id);
    return *blockDefs[id];
}

Item& ContentBuilder::createItem(const std::string& id) {
    auto found = itemDefs.find(id);
    if (found != itemDefs.end()) {
        return *found->second;
    }
    checkIdentifier(id);
    itemIds.push_back(id);
    itemDefs[id] = std::make_unique<Item>(id);
    return *itemDefs[id];
}

BlockMaterial& ContentBuilder::createBlockMaterial(const std::string& id) {
    blockMaterials[id] = std::make_unique<BlockMaterial>();
    auto& material = *blockMaterials[id];
    material.name = id;
    return material;
}

void ContentBuilder::checkIdentifier(const std::string& id) {
    ContentType result;
    if (((result = checkContentType(id)) != ContentType::None)) {
        LOG_ERROR("Name {} is already used", id);
        throw namereuse_error("Name " + id + " is already used", result);
    }  
}

ContentType ContentBuilder::checkContentType(const std::string& id) {
    if (blockDefs.find(id) != blockDefs.end()) return ContentType::Block;
    if (itemDefs.find(id) != itemDefs.end()) return ContentType::Item;

    return ContentType::None;
}

std::unique_ptr<Content> ContentBuilder::build() {
    std::vector<Block*> blockDefsIndices;
    auto groups = std::make_unique<DrawGroups>();
    for (const std::string& name : blockIds) {
        Block& def = *blockDefs[name];
        def.rt.id = blockDefsIndices.size();
        def.rt.emissive = *reinterpret_cast<uint32_t*>(def.emission);
        def.rt.solid = def.model == BlockModel::Cube;
        def.rt.extended = def.size.x > 1 || def.size.y > 1 || def.size.z > 1;

        if (def.rotatable) {
            for (uint i = 0; i < BlockRotProfile::MAX_COUNT; ++i) {
                def.rt.hitboxes[i].reserve(def.hitboxes.size());
                for (AABB aabb : def.hitboxes) {
                    def.rotations.variants[i].transform(aabb);
                    def.rt.hitboxes[i].push_back(aabb);
                }
            }
        }

        blockDefsIndices.push_back(&def);
        groups->insert(def.drawGroup);
    }

    std::vector<Item*> itemDefsIndices;
    for (const std::string& name : itemIds) {
        Item& def = *itemDefs[name];
        def.rt.id = itemDefsIndices.size();
        def.rt.emissive = *reinterpret_cast<uint32_t*>(def.emission);
        itemDefsIndices.push_back(&def);
    }

    auto content = std::make_unique<Content>(
        std::make_unique<ContentIndices>(blockDefsIndices, itemDefsIndices), 
        std::move(groups), 
        std::move(blockDefs), 
        std::move(itemDefs), 
        std::move(packs), 
        std::move(blockMaterials)
    );

    for (Block* def : blockDefsIndices) {
        def->rt.pickingItem = content->requireItem(def->pickingItem).rt.id;
    }

    for (Item* def : itemDefsIndices) {
        def->rt.placingBlock = content->requireBlock(def->placingBlock).rt.id;
    }

    return content;
}
