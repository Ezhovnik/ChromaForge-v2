#include <logic/scripting/lua/libs/api_lua.h>

#include <assets/Assets.h>
#include <coders/png.h>
#include <debug/Logger.h>
#include <engine/Engine.h>
#include <graphics/core/Texture.h>
#include <util/Buffer.h>
#include <coders/cfmodel.h>
#include <graphics/commons/Model.h>
#include <graphics/core/Atlas.h>
#include <logic/scripting/lua/usertypes/lua_type_canvas.h>
#include <assets/AssetsLoader.h>

static debug::Logger logger("lib-assets");

static void load_texture(
    Assets& assets, const ubyte* bytes, size_t size, const std::string& destname
) {
    try {
        assets.store(png::loadTexture(bytes, size), destname);
    } catch (const std::runtime_error& err) {
        logger.error() << err.what();
    }
}

static int l_request_texture(lua::State* L) {
    std::string filename = lua::require_string(L, 1);
    std::string alias = lua::require_string(L, 2);
    auto& loader = scripting::engine->acquireBackgroundLoader();
    loader.add(AssetType::Texture, filename, alias);
    return 0;
}

static int l_load_texture(lua::State* L) {
    auto& assets = scripting::engine->requireAssets();
    if (lua::isstring(L, 3) && lua::require_lstring(L, 3) != "png") {
        throw std::runtime_error("Unsupportd image format");
    }
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
            assets,
            buffer.data(),
            buffer.size(),
            lua::require_string(L, 2)
        );
    } else {
        auto string = lua::bytearray_as_string(L, 1);
        load_texture(
            assets,
            reinterpret_cast<const ubyte*>(string.data()),
            string.size(),
            lua::require_string(L, 2)
        );
        lua::pop(L);
    }
    return 0;
}

static int l_parse_model(lua::State* L) {
    auto& assets = scripting::engine->requireAssets();
    auto format = lua::require_lstring(L, 1);
    auto string = lua::require_lstring(L, 2);
    auto name = lua::require_string(L, 3);

    if (format == "xml" || format == "cfmodel") {
        auto cfModel = cfmodel::parse(name, string, format == "xml");
        assets.store(
            std::make_unique<model::Model>(
                std::move(cfModel.squash())
            ),
            name
        );
    } else {
        throw std::runtime_error("Unknown format " + util::quote(std::string(format)));
    }
    return 0;
}

static int l_to_canvas(lua::State* L) {
    auto& assets = scripting::engine->requireAssets();

    auto alias = lua::require_lstring(L, 1);
    size_t sep = alias.rfind(':');
    if (sep == std::string::npos) {
        auto texture = assets.getShared<Texture>(std::string(alias));
        if (texture != nullptr) {
            auto image = texture->readData();
            return lua::newuserdata<lua::LuaCanvas>(
                L, texture, std::move(image)
            );
        }
        return 0;
    }

    auto atlasName = alias.substr(0, sep);
    if (auto atlas = assets.get<Atlas>(std::string(atlasName))) {
        auto textureName = std::string(alias.substr(sep + 1));
        auto image = atlas->shareImageData();
        auto texture = atlas->shareTexture();
        if (auto region = atlas->getIf(textureName)) {
            UVRegion uvRegion = *region;
            int atlasWidth = static_cast<int>(image->getWidth());
            int atlasHeight = static_cast<int>(image->getHeight());
            int x = static_cast<int>(uvRegion.u1 * atlasWidth);
            int y = static_cast<int>(uvRegion.v1 * atlasHeight);
            int w = static_cast<int>(uvRegion.getWidth() * atlasWidth);
            int h = static_cast<int>(uvRegion.getHeight() * atlasHeight);
            return lua::newuserdata<lua::LuaCanvas>(
                L, std::move(texture), image->cropped(x, y, w, h), uvRegion
            );
        }
    }
    return 0;
}

const luaL_Reg assetslib[] = {
    {"request_texture", lua::wrap<l_request_texture>},
    {"load_texture", lua::wrap<l_load_texture>},
    {"parse_model", lua::wrap<l_parse_model>},
    {"to_canvas", lua::wrap<l_to_canvas>},
    {nullptr, nullptr}
};
