#include "Item.h"

#include "../util/stringutil.h"

Item::Item(std::string name) : name(name) {
    caption = util::id_to_caption(name);
}
