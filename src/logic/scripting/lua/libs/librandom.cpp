#include <logic/scripting/lua/libs/api_lua.h>

#include <math/rand.h>

static int l_random(lua::State* L) {
    int argc = lua::gettop(L);

    if (argc == 0) {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return lua::pushnumber(L, dist(util::RandomGenerator::getGenerator()));
    } else if (argc == 1) {
        integer_t maxVal = lua::tointeger(L, 1);
        return lua::pushinteger(L, util::RandomGenerator::get<integer_t>(1, maxVal));
    } else {
        integer_t minVal = lua::tointeger(L, 1);
        integer_t maxVal = lua::tointeger(L, 2);
        return lua::pushinteger(L, util::RandomGenerator::get<integer_t>(minVal, maxVal));
    }
}

static int l_bytes(lua::State* L) {
    size_t size = lua::tointeger(L, 1);
    std::vector<ubyte> bytes(size);
    for (size_t i = 0; i < size; ++i) {
        bytes[i] = static_cast<ubyte>(util::RandomGenerator::get<int>(0, 0xFF));
    }
    return lua::create_bytearray(L, bytes);
}

static int l_uuid(lua::State* L) {
    return lua::pushlstring(L, util::generate_uuid());
}

const luaL_Reg randomlib[] = {
    {"random", lua::wrap<l_random>},
    {"bytes", lua::wrap<l_bytes>},
    {"uuid", lua::wrap<l_uuid>},
    {NULL, NULL}
};
