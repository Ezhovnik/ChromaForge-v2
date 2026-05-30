#include <logic/scripting/lua/libs/api_lua.h>

#include <engine.h>
#include <network/Network.h>

static int l_get(lua::State* L) {
    std::string url(lua::require_lstring(L, 1));

    lua::pushvalue(L, 2);
    auto onResponse = lua::create_lambda(L);

    scripting::engine->getNetwork().get(url, [onResponse](std::vector<char> bytes) {
        scripting::engine->postRunnable([=]() {
            onResponse({std::string(bytes.data(), bytes.size())});
        });
    });
    return 0;
}

static int l_get_binary(lua::State* L) {
    std::string url(lua::require_lstring(L, 1));

    lua::pushvalue(L, 2);
    auto onResponse = lua::create_lambda(L);

    scripting::engine->getNetwork().get(url, [onResponse](std::vector<char> bytes) {
        auto buffer = std::make_shared<util::Buffer<ubyte>>(
            reinterpret_cast<const ubyte*>(bytes.data()), bytes.size()
        );
        scripting::engine->postRunnable([=]() {
            onResponse({buffer});
        });
    });
    return 0;
}

static int l_connect(lua::State* L) {
    std::string address = lua::require_string(L, 1);
    int port = lua::tointeger(L, 2);
    uint64_t id = scripting::engine->getNetwork().connect(address, port);
    return lua::pushinteger(L, id);
}

const luaL_Reg networklib[] = {
    {"get", lua::wrap<l_get>},
    {"get_binary", lua::wrap<l_get_binary>},
    {"__connect", lua::wrap<l_connect>},
    {NULL, NULL}
};
