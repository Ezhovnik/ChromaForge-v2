#include <logic/scripting/lua/libs/api_lua.h>

#include <io/io.h>
#include <io/util.h>
#include <coders/binary_json.h>
#include <world/Level.h>
#include <world/generator/VoxelFragment.h>
#include <engine/Engine.h>
#include <logic/scripting/lua/lua_custom_types.h>
#include <content/ContentLoader.h>
#include <content/Content.h>
#include <content/ContentControl.h>

static int l_save_fragment(lua::State* L) {
    auto fragment = lua::touserdata<lua::LuaVoxelFragment>(L, 1);
    auto file = lua::require_string(L, 2);
    auto map = fragment->getFragment(0)->serialize();
    auto bytes = json::to_binary(map, true);
    io::write_bytes(file, bytes.data(), bytes.size());
    return 0;
}

static int l_create_fragment(lua::State* L) {
    auto pointA = lua::tovec<3>(L, 1);
    auto pointB = lua::tovec<3>(L, 2);
    bool crop = lua::toboolean(L, 3);
    bool saveEntities = lua::toboolean(L, 4);

    auto fragment = VoxelFragment::create(*scripting::level, pointA, pointB, crop, saveEntities);
    fragment->prepare(*scripting::content);
    std::array<std::shared_ptr<VoxelFragment>, 4> fragmentVariants {
        std::move(fragment)
    };
    for (size_t i = 1; i < 4; ++i) {
        fragmentVariants[i] = fragmentVariants[i - 1]->rotated(*scripting::content);
    }
    return lua::newuserdata<lua::LuaVoxelFragment>(
        L, std::move(fragmentVariants)
    );
}

static int l_load_fragment(lua::State* L) {
    dv::value map;
    if (!lua::isstring(L, 1)) {
        io::path path = lua::require_string(L, 1);
        if (!io::exists(path)) {
            throw std::runtime_error("File " + path.string() + " does not exist");
        }
        map = io::read_binary_json(path);
    } else {
        auto bytearray = lua::bytearray_as_string(L, 1);
        map = json::from_binary(
            reinterpret_cast<const ubyte*>(bytearray.data()), bytearray.size()
        );
    }

    auto fragment = std::make_shared<VoxelFragment>();
    fragment->deserialize(map);
    fragment->prepare(*scripting::content);
    std::array<std::shared_ptr<VoxelFragment>, 4> fragmentVariants {
        std::move(fragment)
    };
    for (size_t i = 1; i < 4; ++i) {
        fragmentVariants[i] = fragmentVariants[i - 1]->rotated(*scripting::content);
    }
    return lua::newuserdata<lua::LuaVoxelFragment>(
        L, std::move(fragmentVariants)
    );
}

static int l_get_generators(lua::State* L) {
    auto packs = scripting::content_control->getAllContentPacks();

    lua::createtable(L, 0, 0);

    for (const auto& pack : packs) {
        auto pairs = ContentLoader::scanContent(pack, ContentType::Generator);
        for (const auto& [name, caption] : pairs) {
            lua::pushstring(L, caption);
            lua::setfield(L, name);
        }
    }
    return 1;
}

static int l_get_default_generator(lua::State* L) {
    auto combined = scripting::engine->getResPaths().readCombinedObject(
        EnginePaths::CONFIG_DEFAULTS.string()
    );
    return lua::pushstring(L, combined["generator"].asString());
}

const luaL_Reg generationlib[] = {
    {"create_fragment", lua::wrap<l_create_fragment>},
    {"save_fragment", lua::wrap<l_save_fragment>},
    {"load_fragment", lua::wrap<l_load_fragment>},
    {"get_generators", lua::wrap<l_get_generators>},
    {"get_default_generator", lua::wrap<l_get_default_generator>},
    {NULL, NULL}
};
