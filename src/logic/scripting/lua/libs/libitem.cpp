#include <logic/scripting/lua/libs/api_lua.h>
#include <content/Content.h>
#include <items/Item.h>

static const Item* get_item_def(lua::State* L, int idx) {
    auto indices = scripting::content->getIndices();
    auto id = lua::tointeger(L, idx);
    return indices->items.get(id);
}

static int l_name(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        return lua::pushstring(L, def->name);
    }
    return 0;
}

static int l_index(lua::State* L) {
    auto name = lua::require_string(L, 1);
    return lua::pushinteger(L, scripting::content->items.require(name).rt.id);
}

static int l_stack_size(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        return lua::pushinteger(L, def->stackSize);
    }
    return 0;
}

static int l_defs_count(lua::State* L) {
    return lua::pushinteger(L, scripting::indices->items.count());
}

static int l_get_icon(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        switch (def->iconType) {
            case ItemIconType::None:
                return 0;
            case ItemIconType::Sprite:
                return lua::pushstring(L, def->icon);
            case ItemIconType::Block:
                return lua::pushstring(L, "block-previews:" + def->icon);
        }
    }
    return 0;
}

static int l_caption(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        return lua::pushstring(L, def->caption);
    }
    return 0;
}

static int l_placing_block(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        return lua::pushinteger(L, def->rt.placingBlock);
    }
    return 0;
}

static int l_model_name(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        return lua::pushstring(L, def->modelName);
    }
    return 0;
}

static int l_emission(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        lua::createtable(L, 4, 0);
        for (int i = 0; i < 4; ++i) {
            lua::pushinteger(L, def->emission[i]);
            lua::rawseti(L, i + 1);
        }
        return 1;
    }
    return 0;
}

const luaL_Reg itemlib [] = {
    {"index", lua::wrap<l_index>},
    {"name", lua::wrap<l_name>},
    {"caption", lua::wrap<l_caption>},
    {"stack_size", lua::wrap<l_stack_size>},
    {"defs_count", lua::wrap<l_defs_count>},
    {"icon", lua::wrap<l_get_icon>},
    {"placing_block", lua::wrap<l_placing_block>},
    {"model_name", lua::wrap<l_model_name>},
    {"emission", lua::wrap<l_emission>},
    {NULL, NULL}
};
