#pragma once

#include <logic/scripting/lua/libs/api_lua.h>

#include <logic/scripting/scripting_hud.h>
#include <graphics/render/WorldRenderer.h>
#include <engine/Engine.h>

namespace lua {
    template <lua_CFunction func>
    inline int wrap_hud(lua_State* L) {
        if (scripting::hud == nullptr) {
            return luaL_error(
                L, "Renderer is not initialized yet, see hud.lua on_hud_open event"
            );
        }
        return lua::wrap<func>(L);
    }
}
