#include "Content.h"

#include <stdexcept>
#include <utility>

#include <glm/glm.hpp>

#include "../debug/Logger.h"
#include "../voxels/Block.h"
#include "../items/Item.h"
#include "ContentPack.h"
#include "../logic/scripting/scripting.h"

ContentIndices::ContentIndices(
    std::vector<Block*> blockDefs, 
    std::vector<Item*> itemDefs
) : blockDefs(std::move(blockDefs)), 
    itemDefs(std::move(itemDefs)) {}

Content::Content(
    std::unique_ptr<ContentIndices> indices,
    std::unique_ptr<DrawGroups> drawGroups, 
    std::unordered_map<std::string, std::unique_ptr<Block>> blockDefs,
    std::unordered_map<std::string, std::unique_ptr<Item>> itemDefs,
    std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>> packs,
    std::unordered_map<std::string, std::unique_ptr<BlockMaterial>> blockMaterials
) : blockDefs(std::move(blockDefs)),
    indices(std::move(indices)),
    drawGroups(std::move(drawGroups)),
    itemDefs(std::move(itemDefs)),
    packs(std::move(packs)),
    blockMaterials(std::move(blockMaterials)) {}

Content::~Content() {
}

Block* Content::findBlock(const std::string& id) const {
    auto found = blockDefs.find(id);
    if (found == blockDefs.end()) return nullptr;
    return found->second.get();
}

Block& Content::requireBlock(const std::string& id) const {
    auto found = blockDefs.find(id);
    if (found == blockDefs.end()) {
        LOG_ERROR("Missing block {}", id);
        throw std::runtime_error("Missing block " + id);
    }
    return *found->second;
}

Item* Content::findItem(const std::string& id) const {
    auto found = itemDefs.find(id);
    if (found == itemDefs.end()) return nullptr;
    return found->second.get();
}

Item& Content::requireItem(const std::string& id) const {
    auto found = itemDefs.find(id);
    if (found == itemDefs.end()) {
        LOG_ERROR("Missing item {}", id);
        throw std::runtime_error("Missing item " + id);
    }
    return *found->second;
}

const ContentPackRuntime* Content::getPackRuntime(const std::string& id) const {
    auto found = packs.find(id);
    if (found == packs.end()) return nullptr;
    return found->second.get();
}

const std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>>& Content::getPacks() const {
    return packs;
}

const BlockMaterial* Content::findBlockMaterial(const std::string& id) const {
    auto found = blockMaterials.find(id);
    if (found == blockMaterials.end()) return nullptr;
    return found->second.get();
}

const std::unordered_map<std::string, std::unique_ptr<BlockMaterial>>& Content::getBlockMaterials() const {
    return blockMaterials;
}
