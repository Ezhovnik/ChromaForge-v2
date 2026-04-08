#include "ContentLoader.h"

#include <string>
#include <memory>
#include <algorithm>

#include <glm/glm.hpp>

#include "Content.h"
#include "ContentPack.h"
#include "../voxels/Block.h"
#include "../files/files.h"
#include "../coders/json.h"
#include "../typedefs.h"
#include "../debug/Logger.h"
#include "../logic/scripting/scripting.h"
#include "../util/listutil.h"
#include "../items/Item.h"
#include "../data/dynamic.h"
#include "../core_content_defs.h"
#include "ContentBuilder.h"
#include "../util/stringutil.h"

ContentLoader::ContentLoader(ContentPack* pack, ContentBuilder& builder) : pack(pack), builder(builder) {
    auto runtime = std::make_unique<ContentPackRuntime>(
        *pack, scripting::create_pack_environment(*pack)
    );
    stats = &runtime->getStatsWriteable();
    env = runtime->getEnvironment();
    builder.add(std::move(runtime));
}

static void detect_defs(
    const std::filesystem::path& folder,
    const std::string& prefix,
    std::vector<std::string>& detected
) {
    if (std::filesystem::is_directory(folder)) {
        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
            const std::filesystem::path& file = entry.path();
            std::string name = file.stem().string();
            if (name[0] == '_') continue;
            if (std::filesystem::is_regular_file(file) && file.extension() == ".json") {
                detected.push_back(prefix.empty() ? name : prefix + ":" + name);
            } else if (std::filesystem::is_directory(file)) {
                detect_defs(file, name, detected);
            }
        }
    }
}

bool ContentLoader::fixPackIndices(
    const std::filesystem::path& folder,
    dynamic::Map* indicesRoot,
    const std::string& contentSection
) {
    std::vector<std::string> detected;
    detect_defs(folder, "", detected);

    std::vector<std::string> indexed;
    bool modified = false;
    if (!indicesRoot->has(contentSection)) {
        indicesRoot->putList(contentSection);
    }
    auto arr = indicesRoot->list(contentSection);
    if (arr) {
        for (uint i = 0; i < arr->size(); ++i) {
            std::string name = arr->str(i);
            if (!util::contains(detected, name)) {
                arr->remove(i);
                i--;
                modified = true;
                continue;
            }
            indexed.push_back(name);
        }
    }
    for (auto name : detected) {
        if (!util::contains(indexed, name)) {
            arr->put(name);
            modified = true;
        }
    }
    return modified;
}

void ContentLoader::fixPackIndices() {
    auto folder = pack->folder;
    auto indexFile = pack->getContentFile();
    auto blocksFolder = folder/ContentPack::BLOCKS_FOLDER;
    auto itemsFolder = folder/ContentPack::ITEMS_FOLDER;
    auto entitiesFolder = folder/ContentPack::ENTITIES_FOLDER;

    dynamic::Map_sptr root;
    if (std::filesystem::is_regular_file(indexFile)) {
        root = files::read_json(indexFile);
    } else {
        root = dynamic::create_map();
    }

    bool modified = false;
    modified |= fixPackIndices(blocksFolder, root.get(), "blocks");
    modified |= fixPackIndices(itemsFolder, root.get(), "items");
    modified |= fixPackIndices(entitiesFolder, root.get(), "entities");

    if (modified) files::write_json(indexFile, root.get());
}

void ContentLoader::loadBlock(Block& def, const std::string& name, const std::filesystem::path& file) {
    auto root = files::read_json(file);

    root->str("caption", def.caption);

    if (root->has("texture")) {
        std::string texture;
        root->str("texture", texture);
        for (uint i = 0; i < 6; ++i) {
            def.textureFaces[i] = texture;
        }
    } else if (root->has("texture-faces")) {
        auto texarr = root->list("texture-faces");
        for (uint i = 0; i < 6; ++i) {
            def.textureFaces[i] = texarr->str(i);
        }
    }

    std::string model = "block";
    root->str("model", model);
    if (model == "block") {
        def.model = BlockModel::Cube;
    } else if (model == "aabb") {
        def.model = BlockModel::AABB;
    } else if (model == "custom") { 
        def.model = BlockModel::Custom;
        if (root->has("model-primitives")) {
            loadCustomBlockModel(def, root->map("model-primitives"));
        } else {
            LOG_ERROR("Error occured while block {} parsed: no 'model-primitives' found", name);
        }
    } else if (model == "X") {
        def.model = BlockModel::X;
    } else if (model == "none") {
        def.model = BlockModel::None;
    } else {
        LOG_WARN("Unknown block {} model {}", name, model);
        def.model = BlockModel::None;
    }

    root->str("material", def.material);

    std::string profile = BlockRotProfile::NONE_NAME;
    root->str("rotation", profile);
    def.rotatable = profile != BlockRotProfile::NONE_NAME;
    if (profile == BlockRotProfile::PIPE_NAME) {
        def.rotations = BlockRotProfile::PIPE;
    } else if (profile == BlockRotProfile::PANE_NAME) {
        def.rotations = BlockRotProfile::PANE;
    } else if (profile != BlockRotProfile::NONE_NAME) {
        LOG_WARN("Unknown block {} rotation profile {}", name, profile);
        def.rotatable = false;
    }

    auto boxarr = root->list("hitboxes");
    if (boxarr) {
        def.hitboxes.resize(boxarr->size());
        for (uint i = 0; i < boxarr->size(); ++i) {
            auto box = boxarr->list(i);
            def.hitboxes[i].a = glm::vec3(box->num(0), box->num(1), box->num(2));
            def.hitboxes[i].b = glm::vec3(box->num(3), box->num(4), box->num(5));
            def.hitboxes[i].b += def.hitboxes[i].a;
        }
    } else if ((boxarr = root->list("hitbox"))) {
        AABB aabb;
        aabb.a = glm::vec3(boxarr->num(0), boxarr->num(1), boxarr->num(2));
        aabb.b = glm::vec3(boxarr->num(3), boxarr->num(4), boxarr->num(5));
        aabb.b += aabb.a;
        def.hitboxes = { aabb };
    } else if (!def.modelBoxes.empty()) {
        def.hitboxes = def.modelBoxes;
    } else {
        def.hitboxes = { AABB() };
    }

    if (auto emissionarr = root->list("emission")) {
        def.emission[0] = emissionarr->num(0);
        def.emission[1] = emissionarr->num(1);
        def.emission[2] = emissionarr->num(2);
    }

    if (auto sizearr = root->list("size")) {
        def.size.x = sizearr->num(0);
        def.size.y = sizearr->num(1);
        def.size.z = sizearr->num(2);
        if (def.model == BlockModel::Cube && (def.size.x != 1 || def.size.y != 1 || def.size.z != 1)) {
            def.model = BlockModel::AABB;
            def.hitboxes = {AABB(def.size)};
        }
    }

    root->flag("obstacle", def.obstacle);
    root->flag("replaceable", def.replaceable);
    root->flag("light-passing", def.lightPassing);
    root->flag("sky-light-passing", def.skyLightPassing);
    root->flag("shadeless", def.shadeless);
    root->flag("breakable", def.breakable);
    root->flag("selectable", def.selectable);
    root->flag("grounded", def.grounded);
    root->flag("hidden", def.hidden);
    root->num("draw-group", def.drawGroup);
    root->str("picking-item", def.pickingItem);
    root->str("script-name", def.scriptName);
    root->str("ui-layout", def.uiLayout);
    root->num("inventory-size", def.inventorySize);
    root->num("tick-interval", def.sparkInterval);
    if (def.sparkInterval == 0) {
        def.sparkInterval = 1;
    }

    if (def.hidden && def.pickingItem == def.name + BLOCK_ITEM_SUFFIX) {
        def.pickingItem = BUILTIN_EMPTY;
    }
}

void ContentLoader::loadCustomBlockModel(Block& def, dynamic::Map* primitives) {
    if (primitives->has("aabbs")) {
        auto modelboxes = primitives->list("aabbs");
        for (uint i = 0; i < modelboxes->size(); ++i) {
            auto boxarr = modelboxes->list(i);
            AABB modelbox;
            modelbox.a = glm::vec3(boxarr->num(0), boxarr->num(1), boxarr->num(2));
            modelbox.b = glm::vec3(boxarr->num(3), boxarr->num(4), boxarr->num(5));
            modelbox.b += modelbox.a;
            def.modelBoxes.push_back(modelbox);

            if (boxarr->size() == 7) {
                for (uint j = 6; j < 12; ++j) {
                    def.modelTextures.push_back(boxarr->str(6));
                }
            } else if (boxarr->size() == 12) {
                for (uint j = 6; j < 12; ++j) {
                    def.modelTextures.push_back(boxarr->str(j));
                }
            } else {
                for (uint j = 6; j < 12; ++j) {
                    def.modelTextures.emplace_back(TEXTURE_NOTFOUND);
                }
            }
        }
    }

    if (primitives->has("tetragons")) {
        auto modeltetragons = primitives->list("tetragons");
        for (uint i = 0; i < modeltetragons->size(); ++i) {
            auto tgonobj = modeltetragons->list(i);
            glm::vec3 p1(tgonobj->num(0), tgonobj->num(1), tgonobj->num(2));
            glm::vec3 xw(tgonobj->num(3), tgonobj->num(4), tgonobj->num(5));
            glm::vec3 yh(tgonobj->num(6), tgonobj->num(7), tgonobj->num(8));
            def.modelExtraPoints.push_back(p1);
            def.modelExtraPoints.push_back(p1 + xw);
            def.modelExtraPoints.push_back(p1 + xw + yh);
            def.modelExtraPoints.push_back(p1 + yh);

            def.modelTextures.push_back(tgonobj->str(9));
        }
    }
}

void ContentLoader::loadItem(Item& def, const std::string& name, const std::filesystem::path& file) {
    auto root = files::read_json(file);
    root->str("caption", def.caption);

    std::string iconTypeStr = "";
    root->str("icon-type", iconTypeStr);
    if (iconTypeStr == "none") {
        def.iconType = ItemIconType::None;
    } else if (iconTypeStr == "block") {
        def.iconType = ItemIconType::Block;
    } else if (iconTypeStr == "sprite") {
        def.iconType = ItemIconType::Sprite;
    } else if (iconTypeStr.length()){
        LOG_WARN("Unknown icon type: {}", iconTypeStr);
    }
    root->str("icon", def.icon);
    root->str("placing-block", def.placingBlock);
    root->str("script-name", def.scriptName);
    root->num("stack-size", def.stackSize);

    if (auto emissionarr = root->list("emission")) {
        def.emission[0] = emissionarr->num(0);
        def.emission[1] = emissionarr->num(1);
        def.emission[2] = emissionarr->num(2);
    }
}

void ContentLoader::loadEntity(Entity& def, const std::string& name, const std::filesystem::path& file) {
    auto root = files::read_json(file);

    if (auto componentsarr = root->list("components")) {
        for (size_t i = 0; i < componentsarr->size(); ++i) {
            def.components.push_back(componentsarr->str(i));
        }
    }

    if (auto boxarr = root->list("hitbox")) {
        def.hitbox = glm::vec3(boxarr->num(0), boxarr->num(1), boxarr->num(2));
    }

    if (auto triggersarr = root->list("triggers")) {
        for (size_t i = 0; i < triggersarr->size(); ++i) {
            if (auto triggerarr = triggersarr->list(i)) {
                auto triggerType = triggerarr->str(0);
                if (triggerType == "aabb") {
                    def.boxTriggers.push_back({i, {
                        {triggerarr->num(1), triggerarr->num(2), triggerarr->num(3)},
                        {triggerarr->num(4), triggerarr->num(5), triggerarr->num(6)}
                    }});
                } else if (triggerType == "radius") {
                    def.radialTriggers.push_back({i, triggerarr->num(1)});
                } else {
                    LOG_WARN("Entity {}: trigger #{} - unknown type {}", name, i, util::quote(triggerType));
                }
            }
        }
    }
}

void ContentLoader::loadEntity(Entity& def, const std::string& full, const std::string& name) {
    auto folder = pack->folder;
    auto configFile = folder/std::filesystem::path("entities/" + name + ".json");
    if (std::filesystem::exists(configFile)) loadEntity(def, full, configFile);

    for (auto& componentName : def.components) {
        auto scriptfile = folder/std::filesystem::path("scripts/components/" + componentName + ".lua");
        if (std::filesystem::is_regular_file(scriptfile)) {
            scripting::load_entity_component(env, componentName, scriptfile);
        }
    }
}

void ContentLoader::loadBlock(Block& def, const std::string& full, const std::string& name) {
    auto folder = pack->folder;
    auto configFile = folder/std::filesystem::path("blocks/" + name + ".json");
    if (std::filesystem::exists(configFile)) loadBlock(def, full, configFile);

    auto scriptfile = folder/std::filesystem::path("scripts/" + def.scriptName + ".lua");
    if (std::filesystem::is_regular_file(scriptfile)) {
        scripting::load_block_script(env, full, scriptfile, def.rt.funcsset);
    }
    if (!def.hidden) {
        auto& item = builder.items.create(full + BLOCK_ITEM_SUFFIX);
        item.generated = true;
        item.caption = def.caption;
        item.iconType = ItemIconType::Block;
        item.icon = full;
        item.placingBlock = full;

        for (uint j = 0; j < 4; j++) {
            item.emission[j] = def.emission[j];
        }
        stats->totalItems++;
    }
}

void ContentLoader::loadItem(Item& def, const std::string& full, const std::string& name) {
    auto folder = pack->folder;
    auto configFile = folder/std::filesystem::path("items/" + name + ".json");
    if (std::filesystem::exists(configFile)) loadItem(def, full, configFile);

    auto scriptfile = folder/std::filesystem::path("scripts/" + def.scriptName + ".lua");
    if (std::filesystem::is_regular_file(scriptfile)) {
        scripting::load_item_script(env, full, scriptfile, def.rt.funcsset);
    }
}

void ContentLoader::loadBlockMaterial(BlockMaterial& def, const std::filesystem::path& file) {
    auto root = files::read_json(file);
    root->str("steps-sound", def.stepsSound);
    root->str("place-sound", def.placeSound);
    root->str("break-sound", def.breakSound);
}

void ContentLoader::load() {
    LOG_INFO("Loading content pack [{}]", pack->id);

    fixPackIndices();

    auto folder = pack->folder;

    std::filesystem::path scriptFile = folder/std::filesystem::path("scripts/world.lua");
    if (std::filesystem::is_regular_file(scriptFile)) {
        scripting::load_world_script(env, pack->id, scriptFile);
    }

    if (!std::filesystem::is_regular_file(pack->getContentFile())) return;

    auto root = files::read_json(pack->getContentFile());

    if (auto blocksarr = root->list("blocks")) {
        for (size_t i = 0; i < blocksarr->size(); ++i) {
            std::string name = blocksarr->str(i);
            auto colon = name.find(':');
            std::string full = colon == std::string::npos ? pack->id + ":" + name : name;
            if (colon != std::string::npos) name[colon] = '/';
            auto& def = builder.blocks.create(full);
            if (colon != std::string::npos) def.scriptName = name.substr(0, colon) + '/' + def.scriptName;
            loadBlock(def, full, name);
            stats->totalBlocks++;
        }
    }

    if (auto itemsarr = root->list("items")) {
        for (size_t i = 0; i < itemsarr->size(); ++i) {
            std::string name = itemsarr->str(i);
            auto colon = name.find(':');
            std::string full = colon == std::string::npos ? pack->id + ":" + name : name;
            if (colon != std::string::npos) name[colon] = '/';
            auto& def = builder.items.create(full);
            if (colon != std::string::npos) def.scriptName = name.substr(0, colon) + '/' + def.scriptName;
            loadItem(def, full, name);
            stats->totalItems++;
        }
    }

    if (auto entitiesarr = root->list("entities")) {
        for (size_t i = 0; i < entitiesarr->size(); ++i) {
            std::string name = entitiesarr->str(i);
            auto colon = name.find(':');
            std::string full = colon == std::string::npos ? pack->id + ":" + name : name;
            if (colon != std::string::npos) name[colon] = '/';
            auto& def = builder.entities.create(full);
            loadEntity(def, full, name);
            stats->totalEntities++;
        }
    }

    std::filesystem::path materialsDir = folder / std::filesystem::u8path("block_materials");
    if (std::filesystem::is_directory(materialsDir)) {
        for (const auto& entry : std::filesystem::directory_iterator(materialsDir)) {
            const std::filesystem::path& file = entry.path();
            std::string name = pack->id + ":" + file.stem().u8string();
            loadBlockMaterial(builder.createBlockMaterial(name), file);
        }
    }

    LOG_INFO("Successfully loaded content pack [{}]", pack->id);
}
