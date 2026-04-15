#include "Item.h"

#include "util/stringutil.h"

Item::Item(const std::string& name) : name(name) {
    caption = util::id_to_caption(name);
}
