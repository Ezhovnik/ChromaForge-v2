#include <logic/scripting/lua/libs/api_lua.h>

#include <logic/scripting/scripting_hud.h>
#include <graphics/render/WorldRenderer.h>
#include <graphics/render/BlockWrapsRenderer.h>

static int l_wrap(lua::State* L) {
    auto position = lua::tovec3(L, 1);
    std::string texture = lua::require_string(L, 2);
    float emission = lua::isnumber(L, 3) ? lua::tonumber(L, 3) : 1.0f;

    return lua::pushinteger(
        L,
        scripting::renderer->blockWraps->add(
            position, std::move(texture), emission
        )
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
        for (int i = 0; i < wrapper->textureFaces.size(); ++i) {
            wrapper->textureFaces[i] = lua::require_string(L, 2);
        }
    }
    return 0;
}

static int l_set_faces(lua::State* L) {
    if (auto wrapper = scripting::renderer->blockWraps->get(lua::tointeger(L, 1))) {
        for (int i = 0; i < wrapper->textureFaces.size(); ++i) {
            if (lua::isnil(L, 2 + i)) {
                if (wrapper->cullingBits & (1 << i)) {
                    wrapper->cullingBits &= ~(1 << i);
                    wrapper->textureFaces[i] = "";
                    wrapper->dirtySides |= (1 << i);
                }
            } else {
                auto texture = lua::require_string(L, 2 + i);;
                if ((wrapper->cullingBits & (1 << i)) == 0x0 || wrapper->textureFaces[i] != texture) {
                    wrapper->cullingBits |= (1 << i);
                    wrapper->textureFaces[i] = texture;
                }
            }
        }
    }
    return 0;
}

const luaL_Reg blockwrapslib[] = {
    {"wrap", lua::wrap<l_wrap>},
    {"unwrap", lua::wrap<l_unwrap>},
    {"set_pos", lua::wrap<l_set_pos>},
    {"set_texture", lua::wrap<l_set_texture>},
    {"set_faces", lua::wrap<l_set_faces>},
    {nullptr, nullptr}
};
