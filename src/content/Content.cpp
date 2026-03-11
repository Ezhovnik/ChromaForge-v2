#include "Content.h"

#include <stdexcept>

#include <glm/glm.hpp>

#include "../voxels/Block.h"
#include "../logger/Logger.h"
#include "../items/Item.h"
#include "ContentPack.h"
#include "../logic/scripting/scripting.h"

ContentBuilder::~ContentBuilder() {
}

void ContentBuilder::add(Block* def) {
    checkIdentifier(def->name);
    blockDefs[def->name] = def;
    blockIds.push_back(def->name);
}

void ContentBuilder::add(Item* def) {
    checkIdentifier(def->name);
    itemDefs[def->name] = def;
    itemIds.push_back(def->name);
}

void ContentBuilder::add(ContentPackRuntime* pack) {
    packs.push_back(std::unique_ptr<ContentPackRuntime>(pack));
}

Block& ContentBuilder::createBlock(std::string id) {
    auto found = blockDefs.find(id);
    if (found != blockDefs.end()) {
        LOG_ERROR("Name {} is already used", id);
        Logger::getInstance().flush();
        throw namereuse_error("Name " + id + " is already used", ContentType::Item);
    }
    Block* block = new Block(id);
    add(block);
    return *block;
}

Item& ContentBuilder::createItem(std::string id) {
    auto found = itemDefs.find(id);
    if (found != itemDefs.end()) {
        if (found->second->generated) return *found->second;
        LOG_ERROR("Name {} is already used", id);
        Logger::getInstance().flush();
        throw namereuse_error("Name " + id + " is already used", ContentType::Item);
    }
    Item* item = new Item(id);
    add(item);
    return *item;
}

void ContentBuilder::checkIdentifier(std::string id) {
    ContentType result;
    if ((result = checkContentType(id)) != ContentType::None) {
        LOG_ERROR("Identifier {} is already used", (int)result);
        Logger::getInstance().flush();
        throw namereuse_error("Identifier " + id + " is already used", result);
    }  
}

ContentType ContentBuilder::checkContentType(std::string id) {
    if (blockDefs.find(id) != blockDefs.end()) return ContentType::Block;
    if (itemDefs.find(id) != itemDefs.end()) return ContentType::Item;
    return ContentType::None;
}

Content* ContentBuilder::build() {
    std::vector<Block*> blockDefsIndices;
    auto groups = std::make_unique<DrawGroups>();
    for (const std::string& name : blockIds) {
        Block* def = blockDefs[name];

        def->rt.id = blockDefsIndices.size();
        def->rt.emissive = *reinterpret_cast<uint32_t*>(def->emission);
        def->rt.solid = def->model == BlockModel::Cube;

        if (def->rotatable) {
            for (uint i = 0; i < BlockRotProfile::MAX_COUNT; ++i) {
                def->rt.hitboxes[i].reserve(def->hitboxes.size());
                for (AABB aabb : def->hitboxes) {
                    def->rotations.variants[i].transform(aabb);
                    def->rt.hitboxes[i].push_back(aabb);
                }
            }
        }

        blockDefsIndices.push_back(def);
        if (groups->find(def->drawGroup) == groups->end()) groups->insert(def->drawGroup);
    }

    std::vector<Item*> itemDefsIndices;
    for (const std::string& name : itemIds) {
        Item* def = itemDefs[name];

        def->rt.id = itemDefsIndices.size();
        def->rt.emissive = *reinterpret_cast<uint32_t*>(def->emission);
        itemDefsIndices.push_back(def);
    }

    ContentIndices* indices = new ContentIndices(blockDefsIndices, itemDefsIndices);
    auto content = std::make_unique<Content>(
        indices, std::move(groups), blockDefs, itemDefs, std::move(packs)
    );

    for (Block* def : blockDefsIndices) {
        def->rt.pickingItem = content->requireItem(def->pickingItem).rt.id;
    }

    for (Item* def : itemDefsIndices) {
        def->rt.placingBlock = content->requireBlock(def->placingBlock).rt.id;
    }

    return content.release();
}

ContentIndices::ContentIndices(std::vector<Block*> blockDefs, std::vector<Item*> itemDefs) : blockDefs(blockDefs), itemDefs(itemDefs) {
}

Content::Content(
    ContentIndices* indices, 
    std::unique_ptr<DrawGroups> drawGroups, 
    std::unordered_map<std::string, Block*> blockDefs, 
    std::unordered_map<std::string, Item*> itemDefs,
    std::vector<std::unique_ptr<ContentPackRuntime>> packs
) : blockDefs(blockDefs), 
    indices(indices), 
    drawGroups(std::move(drawGroups)), 
    itemDefs(itemDefs),
    packs(std::move(packs))
{
}

Content::~Content() {
}

Block* Content::findBlock(std::string id) const {
    auto found = blockDefs.find(id);
    if (found == blockDefs.end()) return nullptr;
    return found->second;
}

Block& Content::requireBlock(std::string id) const {
    auto found = blockDefs.find(id);
    if (found == blockDefs.end()) {
        LOG_ERROR("Missing block {}", id);
        Logger::getInstance().flush();
        throw std::runtime_error("Missing block " + id);
    }
    return *found->second;
}

Item* Content::findItem(std::string id) const {
    auto found = itemDefs.find(id);
    if (found == itemDefs.end()) return nullptr;
    return found->second;
}

Item& Content::requireItem(std::string id) const {
    auto found = itemDefs.find(id);
    if (found == itemDefs.end()) {
        LOG_ERROR("Missing item {}", id);
        Logger::getInstance().flush();
        throw std::runtime_error("Missing item " + id);
    }
    return *found->second;
}


const std::vector<std::unique_ptr<ContentPackRuntime>>& Content::getPacks() const {
    return packs;
}
