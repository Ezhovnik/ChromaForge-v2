#include <iostream>

#include <zlib.h>

#include <logic/scripting/lua/libs/api_lua.h>

int l_print(lua::State* L) {
    int n = lua::gettop(L);
    lua::getglobal(L, "tostring");
    for (int i = 1; i <= n; ++i) {
        lua::pushvalue(L, -1);
        lua::pushvalue(L, i);
        lua::call(L, 1, 1);
        const char* s = lua::tostring(L, -1);
        if (s == NULL) {
            return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));
        }
        if (i > 1) {
            *scripting::output_stream << "\t";
        }
        *scripting::output_stream << s;
        lua::pop(L);
    }
    *scripting::output_stream << std::endl;
    return 0;
}

int l_crc32(lua::State* L) {
    auto value = lua::tointeger(L, 2);
    if (lua::isstring(L, 1)) {
        auto string = lua::tolstring(L, 1);
        return lua::pushinteger(
            L,
            crc32(
                value,
                reinterpret_cast<const ubyte*>(string.data()),
                string.length()
            )
        );
    } else if (lua::istable(L, 1)) {
        std::vector<ubyte> bytes;
        lua::read_bytes_from_table(L, 1, bytes);
        return lua::pushinteger(L, crc32(value, bytes.data(), bytes.size()));
    } else {
        auto string = lua::bytearray_as_string(L, 1);
        auto res = crc32(
            value, reinterpret_cast<const ubyte*>(string.data()), string.size()
        );
        lua::pop(L);
        return lua::pushinteger(L, res);
    }
}
