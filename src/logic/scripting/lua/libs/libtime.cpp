#include <logic/scripting/lua/libs/api_lua.h>
#include <engine.h>
#include <window/Window.h>

static int l_uptime(lua::State* L) {
    return lua::pushnumber(L, scripting::engine->getUptime());
}

static int l_delta(lua::State* L) {
    return lua::pushnumber(L, scripting::engine->getDeltaTime());
}

const luaL_Reg timelib [] = {
    {"uptime", lua::wrap<l_uptime>},
    {"delta", lua::wrap<l_delta>},
    {NULL, NULL}
};
