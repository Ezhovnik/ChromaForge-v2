#include "api_lua.h"
#include "lua_commons.h"
#include "LuaState.h"
#include "../../../coders/json.h"
#include "../../../data/dynamic.h"
#include "../../../debug/Logger.h"

namespace scripting {
    extern lua::LuaState* state;
}

static int l_json_stringify(lua_State* L) {
    auto value = scripting::state->tovalue(1);
    if (auto mapptr = std::get_if<dynamic::Map_sptr>(&value)) {
        bool nice = lua_toboolean(L, 2);
        auto string = json::stringify(mapptr->get(), nice, "  ");
        lua_pushstring(L, string.c_str());
        return 1;
    } else {
        throw std::runtime_error("Table expected");
    }
}

static int l_json_parse(lua_State* L) {
    auto string = scripting::state->requireString(1);
    auto element = json::parse("<string>", string);
    scripting::state->pushvalue(element);
    return 1;
}

const luaL_Reg jsonlib [] = {
    {"tostring", lua_wrap_errors<l_json_stringify>},
    {"parse", lua_wrap_errors<l_json_parse>},
    {NULL, NULL}
};
