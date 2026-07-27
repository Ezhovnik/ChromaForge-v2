#include <logic/scripting/lua/libs/api_lua.h>

#include <logic/scripting/scripting.h>
#include <engine/Engine.h>
#include <engine/EnginePaths.h>
#include <network/Network.h>
#include <util/platform.h>

static int l_start_debug_instance(lua::State* L) {
    int port = lua::tointeger(L, 1);
    if (port == 0) {
        port = scripting::engine->getNetwork().findFreePort();
        if (port == -1) {
            throw std::runtime_error("Could not find free port");
        }
    }
    const auto& paths = scripting::engine->getPaths();

    std::vector<std::string> args {
        "--res", paths.getResourcesFolder().u8string(),
        "--dir", paths.getUserFilesFolder().u8string(),
        "--dbg-server",  "tcp:" + std::to_string(port),
    };
    platform::new_engine_instance(std::move(args));
    return lua::pushinteger(L, port);
}

const luaL_Reg applib[] = {
    {"start_debug_instance", lua::wrap<l_start_debug_instance>},
    // For other functions see libbuiltin.cpp and res/scripts/stdlib.lua
    {nullptr, nullptr}
};
