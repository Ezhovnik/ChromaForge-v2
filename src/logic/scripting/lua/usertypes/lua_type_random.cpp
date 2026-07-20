#include <logic/scripting/lua/lua_custom_types.h>

#include <chrono>

#include <logic/scripting/lua/lua_util.h>

static int l_random(lua::State* L) {
    std::uniform_int_distribution<> dist(0, std::numeric_limits<int>::max());

    auto& rng = lua::require_userdata<lua::LuaRandom>(L, 1).rng;
    size_t n = lua::touinteger(L, 2);
    lua::createtable(L, n, 0);

    for (size_t i = 0; i < n; ++i) {
        lua::pushnumber(L, dist(rng) / (double)std::numeric_limits<int>::max());
        lua::rawseti(L, i + 1);
    }
    return 1;
}

static int l_seed(lua::State* L) {
    lua::require_userdata<lua::LuaRandom>(L, 1).rng = std::mt19937(lua::touinteger(L, 2));
    return 0;
}

static int l_meta_meta_call(lua::State* L) {
    integer_t seed;
    if (lua::isnoneornil(L, 1)) {
        seed = std::chrono::system_clock::now().time_since_epoch().count();
    } else {
        seed = lua::tointeger(L, 1);
    }
    return lua::newuserdata<lua::LuaRandom>(L, seed);
}

int lua::LuaRandom::createMetatable(lua::State* L) {
    createtable(L, 0, 3);

    requireglobal(L, "__chroma_create_random_methods");
    createtable(L, 0, 0);
    pushcfunction(L, wrap<l_random>);
    setfield(L, "random");
    pushcfunction(L, wrap<l_seed>);
    setfield(L, "seed");
    call(L, 1, 1);

    setfield(L, "__index");

    createtable(L, 0, 1);
    pushcfunction(L, wrap<l_meta_meta_call>);
    setfield(L, "__call");
    setmetatable(L);
    return 1;
}
