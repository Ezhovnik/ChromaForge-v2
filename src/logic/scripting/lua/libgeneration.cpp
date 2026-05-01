#include <logic/scripting/lua/api_lua.h>

#include <files/files.h>
#include <files/util.h>
#include <coders/binary_json.h>
#include <world/Level.h>
#include <world/generator/VoxelFragment.h>
#include <engine.h>
#include <logic/scripting/lua/lua_custom_types.h>

static int l_save_structure(lua::State* L) {
    auto pointA = lua::tovec<3>(L, 1);
    auto pointB = lua::tovec<3>(L, 2);
    auto filename = lua::require_string(L, 3);
    if (!files::is_valid_name(filename)) {
        throw std::runtime_error("Invalid file name");
    }
    bool saveEntities = lua::toboolean(L, 4);

    auto structure = VoxelFragment::create(
        scripting::level,
        pointA, pointB,
        saveEntities
    );
    auto map = structure->serialize();

    auto bytes = json::to_binary(map);
    files::write_bytes(
        std::filesystem::u8path(filename),
        bytes.data(),
        bytes.size()
    );
    return 0;
}

static int l_load_structure(lua::State* L) {
    auto paths = scripting::engine->getPaths();
    auto [prefix, filename] = EnginePaths::parsePath(lua::require_string(L, 1));
    auto path = paths->resolve(prefix + ":generators/" + filename + ".vox");
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("File " + path.u8string() + " does not exist");
    }
    auto map = files::read_binary_json(path);

    auto structure = std::make_shared<VoxelFragment>();
    structure->deserialize(map);
    return lua::newuserdata<lua::LuaVoxelStructure>(L, std::move(structure));
}

const luaL_Reg generationlib[] = {
    {"save_structure", lua::wrap<l_save_structure>},
    {"load_structure", lua::wrap<l_load_structure>},
    {NULL, NULL}
};
