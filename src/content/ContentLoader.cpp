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
    if (std::filesystem::is_regular_file(indexFile)) root = std::move(files::read_json(indexFile));
    else root.reset(new dynamic::Map());

    bool modified = false;

    modified |= fixPackIndices(blocksFolder, root.get(), "blocks");
    modified |= fixPackIndices(itemsFolder, root.get(), "items");

    if (modified) files::write_json(indexFile, root.get());
}

void ContentLoader::loadBlock(Block* definition, std::string name, std::filesystem::path file) {
    auto root = files::read_json(file);

    // Текстуры блока
    if (root->has("texture")) {
        std::string texture;
        root->str("texture", texture);
        for (uint i = 0; i < 6; ++i) {
            definition->textureFaces[i] = texture;
        }
    } else if (root->has("texture-faces")) {
        auto texarr = root->list("texture-faces");
        for (uint i = 0; i < 6; ++i) {
            definition->textureFaces[i] = texarr->str(i);
        }
    }

    // Модель блока
    std::string model = "cube";
    root->str("model", model);
    if (model == "cube") definition->model = BlockModel::Cube;
    else if (model == "aabb") definition->model = BlockModel::AABB;
    else if (model == "X") definition->model = BlockModel::X;
    else if (model == "none") definition->model = BlockModel::None;
    else if (model == "custom") { 
        definition->model = BlockModel::Custom;
        if (root->has("model-primitives")) loadCustomBlockModel(definition, root->map("model-primitives"));
        else LOG_ERROR("Error occured while block {} parsed: no \"model-primitives\" found", name);
    } else {
        LOG_WARN("Unknown block {} model {}", name, model);
        definition->model = BlockModel::None;
    }

    // AABB хитбокс блока в формате [x, y, z, width, height, depth]
    auto hitboxobj = root->list("hitbox");
    if (hitboxobj) {
        AABB& aabb = definition->hitbox;
        aabb.a = glm::vec3(hitboxobj->num(0), hitboxobj->num(1), hitboxobj->num(2));
        aabb.b = glm::vec3(hitboxobj->num(3), hitboxobj->num(4), hitboxobj->num(5));
        aabb.b += aabb.a;
    }

    // Профили поворота
    std::string profile = "none";
    root->str("rotation", profile);
    definition->rotatable = profile != "none";
    if (profile == "pipe") {
        definition->rotations = BlockRotProfile::PIPE;
    } else if (profile == "pane") {
        definition->rotations = BlockRotProfile::PANE;
    } else if (profile != "none") {
        LOG_WARN("Unknown block {} rotation profile {}", name, profile);
        definition->rotatable = false;
    }

    // Освещение от блока в формате [r, g, b]
    auto emissionobj = root->list("emission");
    if (emissionobj) {
        definition->emission[0] = emissionobj->num(0);
        definition->emission[1] = emissionobj->num(1);
        definition->emission[2] = emissionobj->num(2);
    }

    // Другие свойства блока
    root->flag("obstacle", definition->obstacle);
    root->flag("replaceable", definition->replaceable);
    root->flag("light-passing", definition->lightPassing);
    root->flag("breakable", definition->breakable);
    root->flag("selectable", definition->selectable);
    root->flag("sky-light-passing", definition->skyLightPassing);
    root->flag("grounded", definition->grounded);
    root->num("draw-group", definition->drawGroup);
    root->flag("hidden", definition->hidden);
    root->str("picking-item", definition->pickingItem);
    root->str("script-name", definition->scriptName);
    root->num("inventory-size", definition->inventorySize);
}

void ContentLoader::loadCustomBlockModel(Block* def, dynamic::Map* primitives) {
    if (primitives->has("aabbs")) {
        auto modelboxes = primitives->list("aabbs");
        for (uint i = 0; i < modelboxes->size(); ++i) {
            auto boxobj = modelboxes->list(i);
            AABB modelbox;
            modelbox.a = glm::vec3(boxobj->num(0), boxobj->num(1), boxobj->num(2));
            modelbox.b = glm::vec3(boxobj->num(3), boxobj->num(4), boxobj->num(5));
            modelbox.b += modelbox.a;
            def->modelBoxes.push_back(modelbox);

            if (boxobj->size() == 7) {
                for (uint i = 6; i < 12; ++i) {
                    def->modelTextures.push_back(boxobj->str(6));
                }
            } else if (boxobj->size() == 12) {
                for (uint i = 6; i < 12; ++i) {
                    def->modelTextures.push_back(boxobj->str(i));
                }
            } else {
                for (uint i = 6; i < 12; ++i) {
                    def->modelTextures.push_back("notfound");
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

            def->modelExtraPoints.push_back(p1);
            def->modelExtraPoints.push_back(p1 + xw);
            def->modelExtraPoints.push_back(p1 + xw + yh);
            def->modelExtraPoints.push_back(p1 + yh);

            def->modelTextures.push_back(tgonobj->str(9));
        }
    }
}

void ContentLoader::loadItem(Item* definition, std::string name, std::filesystem::path file) {
    auto root = files::read_json(file);

    std::string iconTypeStr = "";
    root->str("icon-type", iconTypeStr);
    if (iconTypeStr == "none") definition->iconType = ItemIconType::None;
    else if (iconTypeStr == "block") definition->iconType = ItemIconType::Block;
    else if (iconTypeStr == "sprite") definition->iconType = ItemIconType::Sprite;
    else if (iconTypeStr.length()) LOG_WARN("Unknown icon type: {}", iconTypeStr);

    root->str("icon", definition->icon);
    root->str("placing-block", definition->placingBlock);
    root->str("script-name", definition->scriptName);
    root->num("stack-size", definition->stackSize);

    // Освещение от предмета в формате [r, g, b]
    auto emissionobj = root->list("emission");
    if (emissionobj) {
        definition->emission[0] = emissionobj->num(0);
        definition->emission[1] = emissionobj->num(1);
        definition->emission[2] = emissionobj->num(2);
    }
}

void ContentLoader::loadBlock(Block* definition, std::string full, std::string name) {
    auto folder = pack->folder;

    std::filesystem::path configFile = folder/std::filesystem::path("blocks/" + name + ".json");
    loadBlock(definition, full, configFile);

    std::filesystem::path scriptfile = folder/std::filesystem::path("scripts/" + definition->scriptName + ".lua");
    if (std::filesystem::is_regular_file(scriptfile)) scripting::load_block_script(full, scriptfile, &definition->rt.funcsset);
}

void ContentLoader::loadItem(Item* item, std::string full, std::string name) {
    auto folder = pack->folder;

    std::filesystem::path configFile = folder/std::filesystem::path("items/" + name + ".json");
    loadItem(item, full, configFile);

    std::filesystem::path scriptfile = folder/std::filesystem::path("scripts/" + item->scriptName + ".lua");
    if (std::filesystem::is_regular_file(scriptfile)) scripting::load_item_script(full, scriptfile, &item->rt.funcsset);
}

void ContentLoader::load(ContentBuilder* builder) {
    LOG_INFO("  Loading content pack [{}]", pack->id);

    fixPackIndices();

    auto folder = pack->folder;

    std::filesystem::path scriptFile = folder/std::filesystem::path("scripts/world.lua");
    if (std::filesystem::is_regular_file(scriptFile)) scripting::load_world_script(pack->id, scriptFile);

    if (!std::filesystem::is_regular_file(pack->getContentFile())) return;\

    auto root = files::read_json(pack->getContentFile());
    auto blocksarr = root->list("blocks");

    if (blocksarr) {
        for (uint i = 0; i < blocksarr->size(); ++i) {
            std::string name = blocksarr->str(i);
            std::string full = pack->id + ":" + name;
            auto def = builder->createBlock(full);
            loadBlock(def, full, name);
            if (!def->hidden) {
                auto item = builder->createItem(full + BLOCK_ITEM_SUFFIX);
                item->generated = true;
                item->iconType = ItemIconType::Block;
                item->icon = full;
                item->placingBlock = full;

                for (uint j = 0; j < 4; ++j) {
                    item->emission[j] = def->emission[j];
                }
            }
        }
    }

    auto itemsarr = root->list("items");
    if (itemsarr) {
        for (uint i = 0; i < itemsarr->size(); ++i) {
            std::string name = itemsarr->str(i);
            std::string full = pack->id + ":" + name;
            loadItem(builder->createItem(full), full, name);
        }
    }

    LOG_INFO("  Successfully loaded content pack [{}]", pack->id);
    Logger::getInstance().flush();
}
