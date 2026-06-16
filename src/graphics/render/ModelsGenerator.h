#pragma once

#include <graphics/commons/Model.h>
#include <math/AABB.h>
#include <data/dv.h>

struct Item;
class Assets;
class Content;
class Block;

class ModelsGenerator {
public:
    static model::Model generate(
        const Item& def, const Content& content, const Assets& assets
    );

    static model::Model fromCustom(
        const Assets& assets,
        const std::vector<AABB>& modelBoxes,
        const std::vector<std::string>& modelTextures,
        const std::vector<glm::vec3>& points,
        bool lighting
    );

    static model::Model loadCustomBlockModel(
        const dv::value& primitives, const Assets& assets, bool lighting
    );
};
