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
#include "../objects/rigging.h"

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
    UptrsMap<std::string, ContentPackRuntime> packs,
    UptrsMap<std::string, BlockMaterial> blockMaterials,
    UptrsMap<std::string, rigging::SkeletonConfig> skeletons
) : indices(std::move(indices)),
    drawGroups(std::move(drawGroups)),
    packs(std::move(packs)),
    blocks(std::move(blocks)),
    items(std::move(items)),
    entities(std::move(entities)),
    blockMaterials(std::move(blockMaterials)),
    skeletons(std::move(skeletons)) {}

Content::~Content() {
}

const rigging::SkeletonConfig* Content::getRig(const std::string& id) const {
    auto found = skeletons.find(id);
    if (found == skeletons.end()) return nullptr;
    return found->second.get();
}

const ContentPackRuntime* Content::getPackRuntime(const std::string& id) const {
    auto found = packs.find(id);
    if (found == packs.end()) return nullptr;
    return found->second.get();
}

const UptrsMap<std::string, ContentPackRuntime>& Content::getPacks() const {
    return packs;
}

const BlockMaterial* Content::findBlockMaterial(const std::string& id) const {
    auto found = blockMaterials.find(id);
    if (found == blockMaterials.end()) return nullptr;
    return found->second.get();
}

const UptrsMap<std::string, BlockMaterial>& Content::getBlockMaterials() const {
    return blockMaterials;
}
