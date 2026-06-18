#include <logic/scripting/lua/libs/api_lua.h>

#include <engine/Engine.h>
#include <network/Network.h>
#include <coders/json.h>

static int l_get(lua::State* L) {
    std::string url(lua::require_lstring(L, 1));

    lua::pushvalue(L, 2);
    auto onResponse = lua::create_lambda_nothrow(L);

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
    auto onResponse = lua::create_lambda_nothrow(L);

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

static int l_post(lua::State* L) {
    std::string url(lua::require_lstring(L, 1));
    auto data = lua::tovalue(L, 2);

    lua::pushvalue(L, 3);
    auto onResponse = lua::create_lambda_nothrow(L);

    auto string = json::stringify(data, false);
    scripting::engine->getNetwork().post(url, string, [onResponse](std::vector<char> bytes) {
        auto buffer = std::make_shared<util::Buffer<ubyte>>(
            reinterpret_cast<const ubyte*>(bytes.data()), bytes.size()
        );
        scripting::engine->postRunnable([=]() {
            onResponse({std::string(bytes.data(), bytes.size())});
        });
    });
    return 0;
}

static int l_connect(lua::State* L) {
    std::string address = lua::require_string(L, 1);
    int port = lua::tointeger(L, 2);
    lua::pushvalue(L, 3);
    auto callback = lua::create_lambda_nothrow(L);
    uint64_t id = scripting::engine->getNetwork().connect(address, port, [callback](uint64_t id) {
        scripting::engine->postRunnable([=]() {
            callback({id});
        });
    });
    return lua::pushinteger(L, id);
}

static int l_close(lua::State* L) {
    uint64_t id = lua::tointeger(L, 1);
    if (auto connection = scripting::engine->getNetwork().getConnection(id)) {
        connection->close();
    }
    return 0;
}

static int l_closeserver(lua::State* L) {
    uint64_t id = lua::tointeger(L, 1);
    if (auto server = scripting::engine->getNetwork().getServer(id)) {
        server->close();
    }
    return 0;
}

static int l_send(lua::State* L) {
    uint64_t id = lua::tointeger(L, 1);
    auto connection = scripting::engine->getNetwork().getConnection(id);
    if (connection == nullptr) return 0;
    if (lua::istable(L, 2)) {
        lua::pushvalue(L, 2);
        size_t size = lua::objlen(L, 2);
        util::Buffer<char> buffer(size);
        for (size_t i = 0; i < size; ++i) {
            lua::rawgeti(L, i + 1);
            buffer[i] = lua::tointeger(L, -1);
            lua::pop(L);
        }
        lua::pop(L);
        connection->send(buffer.data(), size);
    } else if (auto bytes = lua::touserdata<lua::LuaBytearray>(L, 2)) {
        connection->send(
            reinterpret_cast<char*>(bytes->data().data()), bytes->data().size()
        );
    } else if (lua::isstring(L, 2)) {
        auto string = lua::tolstring(L, 2);
        connection->send(string.data(), string.length());
    }
    return 0;
}

static int l_recv(lua::State* L) {
    uint64_t id = lua::tointeger(L, 1);
    int length = lua::tointeger(L, 2);
    auto connection = scripting::engine->getNetwork().getConnection(id);
    if (connection == nullptr) return 0;
    length = glm::min(length, connection->available());
    util::Buffer<char> buffer(length);

    int size = connection->recv(buffer.data(), length);
    if (size == -1) {
        return 0;
    }
    if (lua::toboolean(L, 3)) {
        lua::createtable(L, size, 0);
        for (size_t i = 0; i < size; ++i) {
            lua::pushinteger(L, buffer[i] & 0xFF);
            lua::rawseti(L, i + 1);
        }
    } else {
        lua::newuserdata<lua::LuaBytearray>(L, size);
        auto bytearray = lua::touserdata<lua::LuaBytearray>(L, -1);   
        bytearray->data().reserve(size);
        std::memcpy(bytearray->data().data(), buffer.data(), size);
    }
    return 1;
}

static int l_available(lua::State* L) {
    uint64_t id = lua::tointeger(L, 1);
    if (auto connection = scripting::engine->getNetwork().getConnection(id)) {
        return lua::pushinteger(L, connection->available());
    }
    return 0;
}

static int l_open(lua::State* L) {
    int port = lua::tointeger(L, 1);
    lua::pushvalue(L, 2);
    auto callback = lua::create_lambda_nothrow(L);
    uint64_t id = scripting::engine->getNetwork().openServer(port, [callback](uint64_t id) {
        scripting::engine->postRunnable([=]() {
            callback({id});
        });
    });
    return lua::pushinteger(L, id);
}

static int l_is_alive(lua::State* L) {
    uint64_t id = lua::tointeger(L, 1);
    if (auto connection = scripting::engine->getNetwork().getConnection(id)) {
        return lua::pushboolean(
            L, connection->getState() != network::ConnectionState::Closed
        );
    }
    return lua::pushboolean(L, false);
}

static int l_is_connected(lua::State* L) {
    uint64_t id = lua::tointeger(L, 1);
    if (auto connection = scripting::engine->getNetwork().getConnection(id)) {
        return lua::pushboolean(
            L, connection->getState() == network::ConnectionState::Connected
        );
    }
    return lua::pushboolean(L, false);
}

static int l_get_address(lua::State* L) {
    uint64_t id = lua::tointeger(L, 1);
    if (auto connection = scripting::engine->getNetwork().getConnection(id)) {
        lua::pushstring(L, connection->getAddress());
        lua::pushinteger(L, connection->getPort());
        return 2;
    }
    return 0;
}

static int l_is_serveropen(lua::State* L) {
    uint64_t id = lua::tointeger(L, 1);
    if (auto server = scripting::engine->getNetwork().getServer(id)) {
        return lua::pushboolean(L, server->isOpen());
    }
    return lua::pushboolean(L, false);
}

static int l_get_serverport(lua::State* L) {
    uint64_t id = lua::tointeger(L, 1);
    if (auto server = scripting::engine->getNetwork().getServer(id)) {
        return lua::pushinteger(L, server->getPort());
    }
    return 0;
}

static int l_get_total_upload(lua::State* L) {
    return lua::pushinteger(L, scripting::engine->getNetwork().getTotalUpload());
}

static int l_get_total_download(lua::State* L) {
    return lua::pushinteger(L, scripting::engine->getNetwork().getTotalDownload());
}

const luaL_Reg networklib[] = {
    {"get", lua::wrap<l_get>},
    {"get_binary", lua::wrap<l_get_binary>},
    {"post", lua::wrap<l_post>},
    {"get_total_upload", lua::wrap<l_get_total_upload>},
    {"get_total_download", lua::wrap<l_get_total_download>},
    {"__open", lua::wrap<l_open>},
    {"__closeserver", lua::wrap<l_closeserver>},
    {"__connect", lua::wrap<l_connect>},
    {"__close", lua::wrap<l_close>},
    {"__send", lua::wrap<l_send>},
    {"__recv", lua::wrap<l_recv>},
    {"__available", lua::wrap<l_available>},
    {"__is_alive", lua::wrap<l_is_alive>},
    {"__is_connected", lua::wrap<l_is_connected>},
    {"__get_address", lua::wrap<l_get_address>},
    {"__is_serveropen", lua::wrap<l_is_serveropen>},
    {"__get_serverport", lua::wrap<l_get_serverport>},
    {NULL, NULL}
};
