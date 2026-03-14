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
#include "../logger/Logger.h"
#include "../logic/scripting/scripting.h"
#include "../util/listutil.h"
#include "../items/Item.h"
#include "../data/dynamic.h"

ContentLoader::ContentLoader(ContentPack* pack) : pack(pack) {
}

bool ContentLoader::fixPackIndices(std::filesystem::path folder, dynamic::Map* indicesRoot, std::string contentSection) {
    std::vector<std::string> detected;
    std::vector<std::string> indexed;
    if (std::filesystem::is_directory(folder)) {
        for (auto entry : std::filesystem::directory_iterator(folder)) {
            std::filesystem::path file = entry.path();
            if (std::filesystem::is_regular_file(file) && file.extension() == ".json") {
                std::string name = file.stem().string();
                if (name[0] == '_') continue;
                detected.push_back(name);
            } else if (std::filesystem::is_directory(file)) {
                std::string space = file.stem().string();
                if (space[0] == '_') continue;
                for (auto entry : std::filesystem::directory_iterator(file)) {
                    std::filesystem::path file = entry.path();
                    if (std::filesystem::is_regular_file(file) && file.extension() == ".json") {
                        std::string name = file.stem().string();
                        if (name[0] == '_') continue;
                        detected.push_back(space + ':' + name);
                    }
                }
            }
        }
    }

    bool modified = false;
    if (!indicesRoot->has(contentSection)) indicesRoot->putList(contentSection);
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

    std::unique_ptr<dynamic::Map> root;
    if (std::filesystem::is_regular_file(indexFile)) root = files::read_json(indexFile);
    else root.reset(new dynamic::Map());

    bool modified = false;

    modified |= fixPackIndices(blocksFolder, root.get(), "blocks");
    modified |= fixPackIndices(itemsFolder, root.get(), "items");

    if (modified) files::write_json(indexFile, root.get());
}

void ContentLoader::loadBlock(Block& definition, std::string name, std::filesystem::path file) {
    auto root = files::read_json(file);

    root->str("caption", definition.caption);

    // Текстуры блока
    if (root->has("texture")) {
        std::string texture;
        root->str("texture", texture);
        for (uint i = 0; i < 6; ++i) {
            definition.textureFaces[i] = texture;
        }
    } else if (root->has("texture-faces")) {
        auto texarr = root->list("texture-faces");
        for (uint i = 0; i < 6; ++i) {
            definition.textureFaces[i] = texarr->str(i);
        }
    }

    // Модель блока
    std::string model = "cube";
    root->str("model", model);
    if (model == "cube") definition.model = BlockModel::Cube;
    else if (model == "aabb") definition.model = BlockModel::AABB;
    else if (model == "X") definition.model = BlockModel::X;
    else if (model == "none") definition.model = BlockModel::None;
    else if (model == "custom") { 
        definition.model = BlockModel::Custom;
        if (root->has("model-primitives")) loadCustomBlockModel(definition, root->map("model-primitives"));
        else LOG_ERROR("Error occured while block {} parsed: no \"model-primitives\" found", name);
    } else {
        LOG_WARN("Unknown block {} model {}", name, model);
        definition.model = BlockModel::None;
    }

    // AABB хитбокс блока в формате [x, y, z, width, height, depth]
    auto boxarr = root->list("hitboxes");
    if (boxarr) {
        definition.hitboxes.resize(boxarr->size());
        for (uint i = 0; i < boxarr->size(); ++i) {
            auto box = boxarr->list(i);
            definition.hitboxes[i].a = glm::vec3(box->num(0), box->num(1), box->num(2));
            definition.hitboxes[i].b = glm::vec3(box->num(3), box->num(4), box->num(5));
            definition.hitboxes[i].b += definition.hitboxes[i].a;
        }
    } else {
        boxarr = root->list("hitbox");
        if (boxarr) {
            AABB aabb;
            aabb.a = glm::vec3(boxarr->num(0), boxarr->num(1), boxarr->num(2));
            aabb.b = glm::vec3(boxarr->num(3), boxarr->num(4), boxarr->num(5));
            aabb.b += aabb.a;
            definition.hitboxes = {aabb};
        } else if (!definition.modelBoxes.empty()) {
            definition.hitboxes = definition.modelBoxes;
        } else {
            definition.hitboxes = {AABB()};
        }
    }

    root->str("material", definition.material);

    // Профили поворота
    std::string profile = "none";
    root->str("rotation", profile);
    definition.rotatable = profile != "none";
    if (profile == "pipe") {
        definition.rotations = BlockRotProfile::PIPE;
    } else if (profile == "pane") {
        definition.rotations = BlockRotProfile::PANE;
    } else if (profile != "none") {
        LOG_WARN("Unknown block {} rotation profile {}", name, profile);
        definition.rotatable = false;
    }

    // Освещение от блока в формате [r, g, b]
    auto emissionobj = root->list("emission");
    if (emissionobj) {
        definition.emission[0] = emissionobj->num(0);
        definition.emission[1] = emissionobj->num(1);
        definition.emission[2] = emissionobj->num(2);
    }

    // Другие свойства блока
    root->flag("obstacle", definition.obstacle);
    root->flag("replaceable", definition.replaceable);
    root->flag("light-passing", definition.lightPassing);
    root->flag("breakable", definition.breakable);
    root->flag("selectable", definition.selectable);
    root->flag("sky-light-passing", definition.skyLightPassing);
    root->flag("grounded", definition.grounded);
    root->num("draw-group", definition.drawGroup);
    root->flag("hidden", definition.hidden);
    root->str("picking-item", definition.pickingItem);
    root->str("script-name", definition.scriptName);
    root->num("inventory-size", definition.inventorySize);
    root->str("ui-layout", definition.uiLayout);
}

void ContentLoader::loadCustomBlockModel(Block& definition, dynamic::Map* primitives) {
    if (primitives->has("aabbs")) {
        auto modelboxes = primitives->list("aabbs");
        for (uint i = 0; i < modelboxes->size(); ++i) {
            auto boxobj = modelboxes->list(i);
            AABB modelbox;
            modelbox.a = glm::vec3(boxobj->num(0), boxobj->num(1), boxobj->num(2));
            modelbox.b = glm::vec3(boxobj->num(3), boxobj->num(4), boxobj->num(5));
            modelbox.b += modelbox.a;
            definition.modelBoxes.push_back(modelbox);

            if (boxobj->size() == 7) {
                for (uint i = 6; i < 12; ++i) {
                    definition.modelTextures.push_back(boxobj->str(6));
                }
            } else if (boxobj->size() == 12) {
                for (uint i = 6; i < 12; ++i) {
                    definition.modelTextures.push_back(boxobj->str(i));
                }
            } else {
                for (uint i = 6; i < 12; ++i) {
                    definition.modelTextures.push_back("notfound");
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

            definition.modelExtraPoints.push_back(p1);
            definition.modelExtraPoints.push_back(p1 + xw);
            definition.modelExtraPoints.push_back(p1 + xw + yh);
            definition.modelExtraPoints.push_back(p1 + yh);

            definition.modelTextures.push_back(tgonobj->str(9));
        }
    }
}

void ContentLoader::loadItem(Item& definition, std::string name, std::filesystem::path file) {
    auto root = files::read_json(file);

    root->str("caption", definition.caption);

    std::string iconTypeStr = "";
    root->str("icon-type", iconTypeStr);
    if (iconTypeStr == "none") definition.iconType = ItemIconType::None;
    else if (iconTypeStr == "block") definition.iconType = ItemIconType::Block;
    else if (iconTypeStr == "sprite") definition.iconType = ItemIconType::Sprite;
    else if (iconTypeStr.length()) LOG_WARN("Unknown icon type: {}", iconTypeStr);

    root->str("icon", definition.icon);
    root->str("placing-block", definition.placingBlock);
    root->str("script-name", definition.scriptName);
    root->num("stack-size", definition.stackSize);

    // Освещение от предмета в формате [r, g, b]
    auto emissionobj = root->list("emission");
    if (emissionobj) {
        definition.emission[0] = emissionobj->num(0);
        definition.emission[1] = emissionobj->num(1);
        definition.emission[2] = emissionobj->num(2);
    }
}

void ContentLoader::loadBlock(Block& definition, std::string full, std::string name) {
    auto folder = pack->folder;

    std::filesystem::path configFile = folder/std::filesystem::path("blocks/" + name + ".json");
    if (std::filesystem::exists(configFile)) loadBlock(definition, full, configFile);

    std::filesystem::path scriptfile = folder/std::filesystem::path("scripts/" + definition.scriptName + ".lua");
    if (std::filesystem::is_regular_file(scriptfile)) scripting::load_block_script(env, full, scriptfile, definition.rt.funcsset);
}

void ContentLoader::loadItem(Item& item, std::string full, std::string name) {
    auto folder = pack->folder;

    std::filesystem::path configFile = folder/std::filesystem::path("items/" + name + ".json");
    if (std::filesystem::exists(configFile)) loadItem(item, full, configFile);

    std::filesystem::path scriptfile = folder/std::filesystem::path("scripts/" + item.scriptName + ".lua");
    if (std::filesystem::is_regular_file(scriptfile)) scripting::load_item_script(env, full, scriptfile, item.rt.funcsset);
}


BlockMaterial ContentLoader::loadBlockMaterial(std::filesystem::path file, std::string full) {
    auto root = files::read_json(file);
    BlockMaterial material {full};
    root->str("steps-sound", material.stepsSound);
    root->str("place-sound", material.placeSound);
    root->str("break-sound", material.breakSound);
    return material;
}

void ContentLoader::load(ContentBuilder& builder) {
    LOG_INFO("---Loading content pack [{}]", pack->id);

    auto runtime = new ContentPackRuntime(*pack, scripting::create_pack_environment(*pack));
    builder.add(runtime);
    env = runtime->getEnvironment()->getId();
    ContentPackStats& stats = runtime->getStatsWriteable();

    fixPackIndices();

    auto folder = pack->folder;

    std::filesystem::path scriptFile = folder/std::filesystem::path("scripts/world.lua");
    if (std::filesystem::is_regular_file(scriptFile)) scripting::load_world_script(env, pack->id, scriptFile);

    if (!std::filesystem::is_regular_file(pack->getContentFile())) return;

    auto root = files::read_json(pack->getContentFile());
    auto blocksarr = root->list("blocks");

    if (blocksarr) {
        for (uint i = 0; i < blocksarr->size(); ++i) {
            std::string name = blocksarr->str(i);
            auto colon = name.find(':');
            std::string full = colon == std::string::npos ? pack->id + ":" + name : name;
            if (colon != std::string::npos) name[colon] = '/';
            auto& def = builder.createBlock(full);
            if (colon != std::string::npos) def.scriptName = name.substr(0, colon) + '/' + def.scriptName;
            loadBlock(def, full, name);
            stats.totalBlocks++;
            if (!def.hidden) {
                auto& item = builder.createItem(full + BLOCK_ITEM_SUFFIX);
                item.generated = true;
                item.iconType = ItemIconType::Block;
                item.icon = full;
                item.placingBlock = full;
                item.caption = def.caption;

                for (uint j = 0; j < 4; ++j) {
                    item.emission[j] = def.emission[j];
                }
                stats.totalItems++;
            }
        }
    }

    auto itemsarr = root->list("items");
    if (itemsarr) {
        for (uint i = 0; i < itemsarr->size(); ++i) {
            std::string name = itemsarr->str(i);
            auto colon = name.find(':');
            std::string full = colon == std::string::npos ? pack->id + ":" + name : name;
            if (colon != std::string::npos) name[colon] = '/';
            auto& def = builder.createItem(full);
            if (colon != std::string::npos) def.scriptName = name.substr(0, colon) + '/' + def.scriptName;
            loadItem(def, full, name);
            stats.totalItems++;
        }
    }

    std::filesystem::path materialsDir = folder / std::filesystem::u8path("block_materials");
    if (std::filesystem::is_directory(materialsDir)) {
        for (auto entry : std::filesystem::directory_iterator(materialsDir)) {
            std::filesystem::path file = entry.path();
            std::string name = pack->id + ":" + file.stem().u8string();
            builder.add(loadBlockMaterial(file, name));
        }
    }

    LOG_INFO("---Successfully loaded content pack [{}]", pack->id);
}
