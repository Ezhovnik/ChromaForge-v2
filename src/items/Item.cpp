#include <items/Item.h>

#include <util/stringutil.h>

Item::Item(const std::string& name) : name(name) {
    caption = util::id_to_caption(name);
    description = "";
}

void Item::cloneTo(Item& dst) {
    dst.caption = caption;
    dst.description = description;
    dst.stackSize = stackSize;
    dst.generated = generated;
    std::copy(&emission[0], &emission[3], dst.emission);
    dst.iconType = iconType;
    dst.icon = icon;
    dst.placingBlock = placingBlock;
    dst.scriptName = scriptName;
    dst.modelName = modelName;
    dst.uses = uses;
    dst.usesDisplay = usesDisplay;
}
