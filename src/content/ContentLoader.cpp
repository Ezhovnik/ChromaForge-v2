#include "ContentLoader.h"

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "Content.h"
#include "ContentPack.h"
#include "../voxels/Block.h"
#include "../files/files.h"
#include "../coders/json.h"
#include "../typedefs.h"
#include "../logger/Logger.h"
#include "../logic/scripting/scripting.h"

ContentLoader::ContentLoader(ContentPack* pack) : pack(pack) {
}

Block* ContentLoader::loadBlock(std::string name, std::filesystem::path file) {
    std::unique_ptr<json::JObject> root(files::read_json(file));
    std::unique_ptr<Block> definition(new Block(name));

    // Текстуры блока
    if (root->has("texture")) {
        std::string texture;
        root->str("texture", texture);
        for (uint i = 0; i < 6; ++i) {
            definition->textureFaces[i] = texture;
        }
    } else if (root->has("texture-faces")) {
        json::JArray* texarr = root->arr("texture-faces");
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
    else {
        LOG_WARN("Unknown block {} model {}", name, model);
        definition->model = BlockModel::None;
    }
    
    // AABB хитбокс блока в формате [x, y, z, width, height, depth]
    json::JArray* hitboxobj = root->arr("hitbox");
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
    json::JArray* emissionobj = root->arr("emission");
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

    return definition.release();
}

void ContentLoader::load(ContentBuilder* builder) {
    LOG_INFO("  Loading content pack [{}]", pack->id);

    auto folder = pack->folder;
    auto root = files::read_json(pack->getContentFile());

    json::JArray* blocksarr = root->arr("blocks");
    if (blocksarr) {
        for (uint i = 0; i < blocksarr->size(); ++i) {
            std::string name = blocksarr->str(i);
            std::string prefix = pack->id + ":" + name;
            std::filesystem::path blockfile = folder/std::filesystem::path("blocks/" + name + ".json");
            Block* block = loadBlock(prefix, blockfile);
            builder->add(block);

            std::filesystem::path scriptfile = folder/std::filesystem::path("scripts/" + name + ".lua");
            if (std::filesystem::is_regular_file(scriptfile)) scripting::load_block_script(prefix, scriptfile, &block->rt.funcsset);
        }
    }
    LOG_INFO("  Successfully loaded content pack [{}]", pack->id);
    Logger::getInstance().flush();
}
