#include <content/Content.h>

#include <stdexcept>
#include <utility>

#include <glm/glm.hpp>

#include <debug/Logger.h>
#include <voxels/Block.h>
#include <items/Item.h>
#include <objects/Entity.h>
#include <content/ContentPack.h>
#include <logic/scripting/scripting.h>
#include <objects/rigging.h>
#include <world/generator/Generator.h>
#include <world/generator/VoxelFragment.h>

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
    ContentUnitDefs<Generator> generators,
    UptrsMap<std::string, ContentPackRuntime> packs,
    UptrsMap<std::string, BlockMaterial> blockMaterials,
    UptrsMap<std::string, rigging::SkeletonConfig> skeletons,
    ResourceIndicesSet resourceIndices,
    dv::value defaults,
    std::unordered_map<std::string, int> tags
) : indices(std::move(indices)),
    drawGroups(std::move(drawGroups)),
    packs(std::move(packs)),
    blocks(std::move(blocks)),
    items(std::move(items)),
    entities(std::move(entities)),
    generators(std::move(generators)),
    blockMaterials(std::move(blockMaterials)),
    skeletons(std::move(skeletons)),
    defaults(std::move(defaults)),
    tags(std::move(tags))
{
    for (size_t i = 0; i < RESOURCE_TYPES_COUNT; ++i) {
        this->resourceIndices[i] = std::move(resourceIndices[i]);
    }
}

Content::~Content() = default;

const rigging::SkeletonConfig* Content::getSkeleton(const std::string& id) const {
    auto found = skeletons.find(id);
    if (found == skeletons.end()) return nullptr;
    return found->second.get();
}

const rigging::SkeletonConfig& Content::requireSkeleton(const std::string& id) const {
    auto skeleton = getSkeleton(id);
    if (skeleton == nullptr) {
        THROW_ERR("Skeleton '{}' not loaded", id);
    }
    return *skeleton;
}

const ContentPackRuntime* Content::getPackRuntime(const std::string& id) const {
    auto found = packs.find(id);
    if (found == packs.end()) return nullptr;
    return found->second.get();
}

ContentPackRuntime* Content::getPackRuntime(const std::string& id) {
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

const UptrsMap<std::string, rigging::SkeletonConfig>& Content::getSkeletons() const {
    return skeletons;
}

void ResourceIndices::addAlias(const std::string& name, const std::string& alias) {
    size_t index = indexOf(name);
    if (index == MISSING) {
        THROW_ERR("Resource does not exists: {}", name);
    }
    indices[alias] = index;
}
