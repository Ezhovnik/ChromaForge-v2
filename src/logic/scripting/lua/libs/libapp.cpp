#include <logic/scripting/lua/libs/api_lua.h>

#include <logic/scripting/scripting.h>
#include <engine/Engine.h>
#include <engine/EnginePaths.h>
#include <network/Network.h>
#include <util/platform.h>
#include <window/Window.h>
#include <io/io.h>
#include <io/devices/MemoryDevice.h>

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

static int l_focus(lua::State* L) {
    scripting::engine->getWindow().focus();
    return 0;
}

static int l_create_memory_device(lua::State* L) {
    std::string name = lua::require_string(L, 1);
    if (io::get_device(name)) {
        throw std::runtime_error(
            "Entry-point '" + name + "' is already used"
        );
    }
    if (name.find(':') != std::string::npos) {
        throw std::runtime_error("Invalid entry point name");
    }

    io::set_device(name, std::make_unique<io::MemoryDevice>());
    return 0;
}

const luaL_Reg applib[] = {
    {"start_debug_instance", lua::wrap<l_start_debug_instance>},
    {"focus", lua::wrap<l_focus>},
    {"create_memory_device", lua::wrap<l_create_memory_device>},
    // For other functions see libbuiltin.cpp and res/scripts/stdlib.lua
    {nullptr, nullptr}
};
