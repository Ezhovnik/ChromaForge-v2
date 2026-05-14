#include <cmath>
#include <filesystem>
#include <stdexcept>

#include <logic/scripting/lua/libs/api_lua.h>
#include <world/Level.h>
#include <world/World.h>
#include <engine.h>
#include <assets/Assets.h>
#include <assets/AssetsLoader.h>
#include <files/engine_paths.h>

static WorldInfo& require_world_info() {
    if (scripting::level == nullptr) {
        throw std::runtime_error("No world open");
    }
    return scripting::level->getWorld()->getInfo();
}

static int l_get_list(lua::State* L) {
    auto paths = scripting::engine->getPaths();
    auto worlds = paths->scanForWorlds();

    lua::createtable(L, worlds.size(), 0);
    for (size_t i = 0; i < worlds.size(); ++i) {
        lua::createtable(L, 0, 1);

        auto name = worlds[i].filename().u8string();
        lua::pushstring(L, name);
        lua::setfield(L, "name");

        auto assets = scripting::engine->getAssets();
        std::string icon = "world#" + name + ".icon";

        if (!AssetsLoader::loadExternalTexture(assets, icon, {
            worlds[i]/std::filesystem::path("icon.png"),
            worlds[i]/std::filesystem::path("preview.png")
        })) {
            icon = "gui/no_world_icon";
        }
        lua::pushstring(L, icon);
        lua::setfield(L, "icon");

        lua::rawseti(L, i + 1);
    }
    return 1;
}

static int l_get_total_time(lua::State* L) {
    return lua::pushnumber(L, require_world_info().totalTime);
}

static int l_get_day_time(lua::State* L) {
    return lua::pushnumber(L, require_world_info().daytime);
}

static int l_set_day_time(lua::State* L) {
    auto value = lua::tonumber(L, 1);
    require_world_info().daytime = std::fmod(value, 1.0);
    return 0;
}

static int l_set_day_time_speed(lua::State* L) {
    auto value = lua::tonumber(L, 1);
    require_world_info().daytimeSpeed = std::abs(value);
    return 0;
}

static int l_get_day_time_speed(lua::State* L) {
    return lua::pushnumber(L, require_world_info().daytimeSpeed);
}

static int l_get_seed(lua::State* L) {
    return lua::pushinteger(L, require_world_info().seed);
}

static int l_exists(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto worldsDir = scripting::engine->getPaths()->getWorldFolderByName(name);
    return lua::pushboolean(L, std::filesystem::is_directory(worldsDir));
}

static int l_is_day(lua::State* L) {
    auto daytime = require_world_info().daytime;
    return lua::pushboolean(L, daytime >= 0.2 && daytime <= 0.8);
}

static int l_is_night(lua::State* L) {
    auto daytime = require_world_info().daytime;
    return lua::pushboolean(L, daytime < 0.2 || daytime > 0.8);
}

static int l_get_generator(lua::State* L) {
    return lua::pushstring(L, require_world_info().generator);
}

static int l_is_open(lua::State* L) {
    return lua::pushboolean(L, scripting::level != nullptr);
}

const luaL_Reg worldlib [] = {
    {"is_open", lua::wrap<l_is_open>},
    {"get_list", lua::wrap<l_get_list>},
    {"get_total_time", lua::wrap<l_get_total_time>},
    {"get_day_time", lua::wrap<l_get_day_time>},
    {"set_day_time", lua::wrap<l_set_day_time>},
    {"set_day_time_speed", lua::wrap<l_set_day_time_speed>},
    {"get_day_time_speed", lua::wrap<l_get_day_time_speed>},
    {"get_seed", lua::wrap<l_get_seed>},
    {"is_day", lua::wrap<l_is_day>},
    {"is_night", lua::wrap<l_is_night>},
    {"exists", lua::wrap<l_exists>},
    {"get_generator", lua::wrap<l_get_generator>},
    {NULL, NULL}
};
