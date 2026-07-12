#include <logic/scripting/lua/libs/api_lua.h>

#include <assets/Assets.h>
#include <coders/png.h>
#include <debug/Logger.h>
#include <engine/Engine.h>
#include <graphics/core/Texture.h>
#include <util/Buffer.h>
#include <coders/cfmodel.h>
#include <graphics/commons/Model.h>

static void load_texture(
    const ubyte* bytes, size_t size, const std::string& destname
) {
    try {
        scripting::engine->getAssets()->store(png::loadTexture(bytes, size), destname);
    } catch (const std::runtime_error& err) {
        LOG_ERROR("{}", err.what());
    }
}

static int l_load_texture(lua::State* L) {
    if (lua::istable(L, 1)) {
        lua::pushvalue(L, 1);
        size_t size = lua::objlen(L, 1);
        util::Buffer<ubyte> buffer(size);
        for (size_t i = 0; i < size; ++i) {
            lua::rawgeti(L, i + 1);
            buffer[i] = lua::tointeger(L, -1);
            lua::pop(L);
        }
        lua::pop(L);
        load_texture(
            buffer.data(),
            buffer.size(),
            lua::require_string(L, 2)
        );
    } else {
        auto string = lua::bytearray_as_string(L, 1);
        load_texture(
            reinterpret_cast<const ubyte*>(string.data()),
            string.size(),
            lua::require_string(L, 2)
        );
        lua::pop(L);
    }
    return 0;
}

static int l_parse_model(lua::State* L) {
    auto format = lua::require_lstring(L, 1);
    auto string = lua::require_lstring(L, 2);
    auto name = lua::require_string(L, 3);

    if (format == "xml") {
        scripting::engine->getAssets()->store(cfmodel::parse(name, string), name);
    } else {
        throw std::runtime_error("Unknown format " + util::quote(std::string(format)));
    }
    return 0;
}

const luaL_Reg assetslib[] = {
    {"load_texture", lua::wrap<l_load_texture>},
    {"parse_model", lua::wrap<l_parse_model>},
    {NULL, NULL}
};
