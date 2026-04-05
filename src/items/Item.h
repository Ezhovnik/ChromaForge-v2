#ifndef CONTENT_ITEMS_ITEM_H_
#define CONTENT_ITEMS_ITEM_H_

#include <string>
#include <glm/glm.hpp>

#include "../math/UVRegion.h"
#include "../typedefs.h"
#include "../core_content_defs.h"

struct item_funcs_set {
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

struct Item {
    std::string const name;

    std::string caption;

    itemcount_t stackSize = 64;

    bool generated = false;

    ubyte emission[4] {0, 0, 0, 0};

    ItemIconType iconType = ItemIconType::Sprite;
    std::string icon = "blocks:notfound";

    std::string placingBlock = BUILTIN_AIR;
    std::string scriptName = name.substr(name.find(':') + 1);

    struct {
        itemid_t id;
        item_funcs_set funcsset {};
        blockid_t placingBlock;
        bool emissive = false;
    } rt;

    Item(const std::string& name);
    Item(const Item&) = delete;
};

#endif //CONTENT_ITEMS_ITEM_H_
