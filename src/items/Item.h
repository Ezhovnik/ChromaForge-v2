#ifndef CONTENT_ITEMS_ITEM_H_
#define CONTENT_ITEMS_ITEM_H_

#include <string>
#include <glm/glm.hpp>

#include "../graphics/UVRegion.h"
#include "../typedefs.h"

struct item_funcs_set {
	bool init: 1;
};

enum class ItemIconType {
    None,
    Sprite,
    Block
};

class Item {
public:
    std::string const name;

    bool generated = false;

    ubyte emission[4] {0, 0, 0, 0};

    ItemIconType iconType = ItemIconType::Sprite;
    std::string icon = "block:notfound";

    std::string placingBlock = "none";

    struct {
        itemid_t id;
        item_funcs_set funcsset {};
        UVRegion iconRegion {0, 0, 1, 1};
        bool emissive = false;
    } rt;

    Item(std::string name) : name(name) {};
};

#endif //CONTENT_ITEMS_ITEM_H_
