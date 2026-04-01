#include "api_lua.h"
#include "lua_commons.h"
#include "LuaState.h"
#include "../../../coders/toml.h"
#include "../../../data/dynamic.h"

namespace scripting {
    extern lua::LuaState* state;
}

static int l_toml_stringify(lua_State* L) {
    auto value = scripting::state->tovalue(1);

    if (auto mapptr = std::get_if<dynamic::Map_sptr>(&value)) {
        auto string = toml::stringify(**mapptr);
        lua_pushstring(L, string.c_str());
        return 1;
    } else {
        throw std::runtime_error("Table expected");
    }
}

static int l_toml_parse(lua_State*) {
    auto string = scripting::state->requireString(1);
    auto element = toml::parse("<string>", string);
    auto value = std::make_unique<dynamic::Value>(element);
    scripting::state->pushvalue(*value);
    return 1;
}

const luaL_Reg tomllib [] = {
    {"tostring", lua_wrap_errors<l_toml_stringify>},
    {"parse", lua_wrap_errors<l_toml_parse>},
    {NULL, NULL}
};
