#define CHROMA_ENABLE_REFLECTION
#include <content/loading/ContentUnitLoader.h>
#include <content/loading/ContentLoadingCommons.h>

#include <content/ContentBuilder.h>
#include <coders/json.h>
#include <core_content_defs.h>
#include <data/dv.h>
#include <debug/Logger.h>
#include <io/io.h>
#include <util/stringutil.h>
#include <items/Item.h>

template<> void ContentUnitLoader<Item>::loadUnit(
    Item& def, const std::string& name, const io::path& file
) {
    auto root = io::read_json(file);

    process_properties(def, name, root);
    process_tags(def, root);

    if (root.has("parent")) {
        const auto& parentName = root["parent"].asString();
        auto parentDef = builder.get(parentName);
        if (parentDef == nullptr) {
            LOG_ERROR("Failed to find parent ({}) for {}", parentName, name);
            throw std::runtime_error(
                "Failed to find parent (" + parentName + ") for " + name
            );
        }
        parentDef->cloneTo(def);
    }
    root.at("caption").get(def.caption);
    root.at("description").get(def.description);

    std::string iconTypeStr = "";
    root.at("icon-type").get(iconTypeStr);
    if (iconTypeStr == "none") {
        def.iconType = ItemIconType::None;
    } else if (iconTypeStr == "block") {
        def.iconType = ItemIconType::Block;
    } else if (iconTypeStr == "sprite") {
        def.iconType = ItemIconType::Sprite;
    } else if (iconTypeStr.length()) {
        LOG_ERROR("Item {}: unknown icon type — {}", name, iconTypeStr);
    }
    root.at("icon").get(def.icon);
    root.at("placing-block").get(def.placingBlock);
    root.at("script-name").get(def.scriptName);
    root.at("model-name").get(def.modelName);
    root.at("stack-size").get(def.stackSize);
    root.at("uses").get(def.uses);

    std::string usesDisplayStr = "";
    root.at("uses-display").get(usesDisplayStr);
    if (usesDisplayStr == "none") {
        def.usesDisplay = ItemUsesDisplay::None;
    } else if (usesDisplayStr == "number") {
        def.usesDisplay = ItemUsesDisplay::Number;
    } else if (usesDisplayStr == "relation") {
        def.usesDisplay = ItemUsesDisplay::Relation;
    } else if (usesDisplayStr == "vbar") {
        def.usesDisplay = ItemUsesDisplay::VBar;
    } else if (usesDisplayStr.length()) {
        LOG_ERROR("Item {}: unknown uses display mode — {}", usesDisplayStr);
    }

    if (auto found = root.at("emission")) {
        const auto& emissionarr = *found;
        def.emission[0] = emissionarr[0].asNumber();
        def.emission[1] = emissionarr[1].asNumber();
        def.emission[2] = emissionarr[2].asNumber();
    }

    def.scriptFile = pack.id + ":scripts/" + def.scriptName + ".lua";
}
