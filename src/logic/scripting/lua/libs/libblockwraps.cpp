#include <logic/scripting/lua/libs/api_lua.h>

#include <logic/scripting/scripting_hud.h>
#include <graphics/render/WorldRenderer.h>
#include <graphics/render/BlockWrapsRenderer.h>

static int l_wrap(lua::State* L) {
    auto position = lua::tovec3(L, 1);
    std::string texture = lua::require_string(L, 2);

    return lua::pushinteger(
        L, scripting::renderer->blockWraps->add(position, std::move(texture))
    );
}

static int l_unwrap(lua::State* L) {
    scripting::renderer->blockWraps->remove(lua::tointeger(L, 1));
    return 0;
}

static int l_set_pos(lua::State* L) {
    if (auto wrapper = scripting::renderer->blockWraps->get(lua::tointeger(L, 1))) {
        wrapper->position = lua::tovec3(L, 2);
    }
    return 0;
}

static int l_set_texture(lua::State* L) {
    if (auto wrapper = scripting::renderer->blockWraps->get(lua::tointeger(L, 1))) {
        wrapper->texture = lua::require_string(L, 2);
    }
    return 0;
}

const luaL_Reg blockwrapslib[] = {
    {"wrap", lua::wrap<l_wrap>},
    {"unwrap", lua::wrap<l_unwrap>},
    {"set_pos", lua::wrap<l_set_pos>},
    {"set_texture", lua::wrap<l_set_texture>},
    {NULL, NULL}
};
