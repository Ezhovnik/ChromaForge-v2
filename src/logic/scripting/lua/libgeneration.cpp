#include <logic/scripting/lua/api_lua.h>

#include <files/files.h>
#include <files/util.h>
#include <coders/binary_json.h>
#include <world/Level.h>
#include <world/generator/Structure.h>

static int l_save_structure(lua::State* L) {
    auto pointA = lua::tovec<3>(L, 1);
    auto pointB = lua::tovec<3>(L, 2);
    auto filename = lua::require_string(L, 3);
    if (!files::is_valid_name(filename)) {
        throw std::runtime_error("Invalid file name");
    }
    bool saveEntities = lua::toboolean(L, 4);

    auto structure = Structure::create(
        scripting::level,
        pointA, pointB,
        saveEntities
    );
    auto map = structure->serialize();

    auto bytes = json::to_binary(map.get());
    files::write_bytes(
        std::filesystem::u8path(filename),
        bytes.data(),
        bytes.size()
    );
    return 0;
}

const luaL_Reg generationlib[] = {
    {"save_structure", lua::wrap<l_save_structure>},
    {NULL, NULL}
};
