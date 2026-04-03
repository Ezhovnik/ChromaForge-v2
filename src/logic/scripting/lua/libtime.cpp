#include "api_lua.h"
#include "../../../engine.h"
#include "../../../window/Window.h"

static int l_time_uptime(lua::State* L) {
    return lua::pushnumber(L, Window::time());
}

static int l_time_delta(lua::State* L) {
    return lua::pushnumber(L, scripting::engine->getDeltaTime());
}

const luaL_Reg timelib [] = {
    {"uptime", lua::wrap<l_time_uptime>},
    {"delta", lua::wrap<l_time_delta>},
    {NULL, NULL}
};
