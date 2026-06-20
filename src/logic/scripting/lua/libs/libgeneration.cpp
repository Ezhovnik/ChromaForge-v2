#include <logic/scripting/lua/libs/api_lua.h>

#include <io/io.h>
#include <io/util.h>
#include <coders/binary_json.h>
#include <world/Level.h>
#include <world/generator/VoxelFragment.h>
#include <engine/Engine.h>
#include <logic/scripting/lua/lua_custom_types.h>
#include <content/ContentLoader.h>

static int l_save_fragment(lua::State* L) {
    const auto& paths = scripting::engine->getPaths();
    auto fragment = lua::touserdata<lua::LuaVoxelFragment>(L, 1);
    auto file = paths.resolve(lua::require_string(L, 2), true);
    auto map = fragment->getFragment()->serialize();
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
    return lua::newuserdata<lua::LuaVoxelFragment>(
        L, std::shared_ptr<VoxelFragment>(std::move(fragment))
    );
}

static int l_load_fragment(lua::State* L) {
    const auto& paths = scripting::engine->getPaths();
    auto filename = lua::require_string(L, 1);
    auto path = paths.resolve(filename);
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("File " + path.u8string() + " does not exist");
    }
    auto map = io::read_binary_json(path);

    auto fragment = std::make_shared<VoxelFragment>();
    fragment->deserialize(map);
    fragment->prepare(*scripting::content);
    return lua::newuserdata<lua::LuaVoxelFragment>(L, std::move(fragment));
}

static int l_get_generators(lua::State* L) {
    auto packs = scripting::engine->getAllContentPacks();

    lua::createtable(L, 0, 0);

    int i = 1;
    for (const auto& pack : packs) {
        auto pairs = ContentLoader::scanContent(pack, ContentType::Generator);
        for (const auto& [name, caption] : pairs) {
            lua::pushstring(L, caption);
            lua::setfield(L, name);
            ++i;
        }
    }
    return 1;
}

static int l_get_default_generator(lua::State* L) {
    auto combined = scripting::engine->getResPaths()->readCombinedObject(
        EnginePaths::CONFIG_DEFAULTS.u8string()
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
