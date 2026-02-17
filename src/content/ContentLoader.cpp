#include "ContentLoader.h"

#include <string>
#include <memory>

#include "Content.h"
#include "../voxels/Block.h"
#include "../files/files.h"
#include "../coders/json.h"
#include "../typedefs.h"
#include "../logger/Logger.h"

#include <glm/glm.hpp>

ContentLoader::ContentLoader(std::filesystem::path folder) : folder(folder) {
}

Block* ContentLoader::loadBlock(std::string name, std::filesystem::path file) {
    std::string source = files::read_string(file);
    std::unique_ptr<json::JObject> root = nullptr;
    try {
        root.reset(json::parse(file.string(), source));
    } catch (const parsing_error& error) {
        LOG_ERROR("Could not load block '{}' definition. Reason: {}", name, error.errorLog());
    }
    std::unique_ptr<Block> definition(new Block(name));

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
    
    json::JArray* hitboxobj = root->arr("hitbox");
    if (hitboxobj) {
        AABB& aabb = definition->hitbox;
        aabb.a = glm::vec3(hitboxobj->num(0), hitboxobj->num(1), hitboxobj->num(2));
        aabb.b = glm::vec3(hitboxobj->num(3), hitboxobj->num(4), hitboxobj->num(5));
        aabb.b += aabb.a;
    }

    json::JArray* emissionobj = root->arr("emission");
    if (emissionobj) {
        definition->emission[0] = emissionobj->num(0);
        definition->emission[1] = emissionobj->num(1);
        definition->emission[2] = emissionobj->num(2);
    }

    root->flag("obstacle", definition->obstacle);
    root->flag("replaceable", definition->replaceable);
    root->flag("light-passing", definition->lightPassing);
    root->flag("breakable", definition->breakable);
    root->flag("selectable", definition->selectable);
    root->flag("rotatable", definition->rotatable);
    root->flag("sky-light-passing", definition->skyLightPassing);
    root->num("draw-group", definition->drawGroup);

    return definition.release();
}

void ContentLoader::load(ContentBuilder* builder) {
    LOG_INFO("Loading content");

    std::filesystem::path file = folder/std::filesystem::path("package.json");
    std::string source = files::read_string(file);

    std::unique_ptr<json::JObject> root = nullptr;
    try {
        root.reset(json::parse(file.filename().string(), source));
    } catch (const parsing_error& error) {
        LOG_ERROR("Could not load content package. Reason: {}", error.errorLog());
    }

    std::string id;
    std::string version;
    root->str("id", id);
    root->str("version", version);

    LOG_INFO("  Content id: {}", id);
    LOG_INFO("  Content version: {}", version);

    json::JArray* blocksarr = root->arr("blocks");
    if (blocksarr) {
        LOG_INFO("  Content blocks size: {}", blocksarr->size());
        for (uint i = 0; i < blocksarr->size(); ++i) {
            std::string name = blocksarr->str(i);
            LOG_DEBUG(" Loading block {}:{}", id, name);
            std::filesystem::path blockfile = folder/std::filesystem::path("blocks/" + name + ".json");
            builder->add(loadBlock(id + ":" + name, blockfile));
        }
    }
}
