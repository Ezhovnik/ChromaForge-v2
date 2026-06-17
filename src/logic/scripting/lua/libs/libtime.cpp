#include <logic/scripting/lua/libs/api_lua.h>
#include <engine/Engine.h>

static int l_uptime(lua::State* L) {
    return lua::pushnumber(L, scripting::engine->getTime().getTime());
}

static int l_delta(lua::State* L) {
    return lua::pushnumber(L, scripting::engine->getTime().getDeltaTime());
}

const luaL_Reg timelib [] = {
    {"uptime", lua::wrap<l_uptime>},
    {"delta", lua::wrap<l_delta>},
    {NULL, NULL}
};
