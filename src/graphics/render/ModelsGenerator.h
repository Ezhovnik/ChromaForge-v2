#pragma once

#include <graphics/commons/Model.h>

struct Item;
class Assets;
class Content;

class ModelsGenerator {
public:
    static model::Model generate(
        const Item& def, const Content& content, const Assets& assets
    );
};
