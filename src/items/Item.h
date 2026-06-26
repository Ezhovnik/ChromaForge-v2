#pragma once

#include <string>
#include <glm/glm.hpp>

#include <math/UVRegion.h>
#include <typedefs.h>
#include <core_content_defs.h>
#include <data/dv.h>

struct ItemFuncsSet {
	bool init: 1;
    bool on_use: 1;
    bool on_use_on_block: 1;
    bool on_block_break_by: 1;
};

enum class ItemIconType {
    None,
    Sprite,
    Block
};

enum class ItemUsesDisplay {
    None,
    Number,
    Relation,
    VBar,
    Default = VBar,
};

struct Item {
    std::string const name;

    std::string caption;

    dv::value properties = nullptr;

    itemcount_t stackSize = 64;

    bool generated = false;

    ubyte emission[4] {0, 0, 0, 0};

    int16_t uses = -1;
    ItemUsesDisplay usesDisplay = ItemUsesDisplay::Default;

    ItemIconType iconType = ItemIconType::Sprite;
    std::string icon = "blocks:notfound";

    std::string placingBlock = BUILTIN_AIR;
    std::string scriptName = name.substr(name.find(':') + 1);
    std::string scriptFile;

    std::string modelName = name + ".model";

    struct {
        itemid_t id;
        blockid_t placingBlock;
        ItemFuncsSet funcsset {};
        bool emissive = false;
    } rt {};

    Item(const std::string& name);
    Item(const Item&) = delete;

    void cloneTo(Item& dst);
};
