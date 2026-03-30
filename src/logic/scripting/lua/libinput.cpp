#include "api_lua.h"
#include "lua_commons.h"
#include "../scripting.h"
#include "../../../window/input.h"
#include "../../../window/Events.h"
#include "../../../frontend/screens/Screen.h"
#include "../../../engine.h"
#include "LuaState.h"
#include "../../../frontend/hud.h"
#include "../../../util/stringutil.h"
#include "lua_util.h"

namespace scripting {
    extern lua::LuaState* state;
    extern Hud* hud;
}

static int l_keycode(lua_State* L) {
    const char* name = lua_tostring(L, 1);
    lua_pushinteger(L, static_cast<int>(input_util::keycode_from(name)));
    return 1;
}

static int l_add_callback(lua_State* L) {
    auto bindname = lua_tostring(L, 1);
    const auto& bind = Events::bindings.find(bindname);
    if (bind == Events::bindings.end()) {
        throw std::runtime_error("Unknown binding: " + util::quote(bindname));
    }
    scripting::state->pushvalue(2);
    runnable callback = scripting::state->createRunnable();
    if (scripting::hud) {
        scripting::hud->keepAlive(bind->second.onactived.add(callback));
    } else {
        scripting::engine->keepAlive(bind->second.onactived.add(callback));
    }
    return 0;
}

static int l_get_mouse_pos(lua_State* L) {
    return lua::pushvec2_arr(L, Events::cursor);
}

const luaL_Reg inputlib [] = {
    {"keycode", lua_wrap_errors<l_keycode>},
    {"add_callback", lua_wrap_errors<l_add_callback>},
    {"get_mouse_pos", lua_wrap_errors<l_get_mouse_pos>},
    {NULL, NULL}
};
