#include "Content.h"

#include <stdexcept>
#include <utility>

#include <glm/glm.hpp>

#include "../debug/Logger.h"
#include "../voxels/Block.h"
#include "../items/Item.h"
#include "../objects/Entity.h"
#include "ContentPack.h"
#include "../logic/scripting/scripting.h"

ContentIndices::ContentIndices(
    ContentUnitIndices<Block> blocks,
    ContentUnitIndices<Item> items,
    ContentUnitIndices<Entity> entities
) : blocks(std::move(blocks)),
    items(std::move(items)),
    entities(std::move(entities)) {}

Content::Content(
    std::unique_ptr<ContentIndices> indices,
    std::unique_ptr<DrawGroups> drawGroups, 
    ContentUnitDefs<Block> blocks,
    ContentUnitDefs<Item> items,
    ContentUnitDefs<Entity> entities,
    std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>> packs,
    std::unordered_map<std::string, std::unique_ptr<BlockMaterial>> blockMaterials
) : indices(std::move(indices)),
    drawGroups(std::move(drawGroups)),
    packs(std::move(packs)),
    blocks(std::move(blocks)),
    items(std::move(items)),
    entities(std::move(entities)),
    blockMaterials(std::move(blockMaterials)) {}

Content::~Content() {
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
