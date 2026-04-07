#include <cmath>
#include <filesystem>

#include "api_lua.h"
#include "../../../world/Level.h"
#include "../../../world/World.h"
#include "../../../engine.h"
#include "../../../assets/Assets.h"
#include "../../../assets/AssetsLoader.h"
#include "../../../files/engine_paths.h"

static int l_world_get_list(lua::State* L) {
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

static int l_world_get_total_time(lua::State* L) {
    return lua::pushnumber(L, scripting::level->getWorld()->totalTime);
}

static int l_world_get_day_time(lua::State* L) {
    return lua::pushnumber(L, scripting::level->getWorld()->daytime);
}

static int l_world_set_day_time(lua::State* L) {
    auto value = lua::tonumber(L, 1);
    scripting::level->getWorld()->daytime = fmod(value, 1.0);
    return 0;
}

static int l_world_get_seed(lua::State* L) {
    return lua::pushinteger(L, scripting::level->getWorld()->getSeed());
}

static int l_world_exists(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto worldsDir = scripting::engine->getPaths()->getWorldFolder(name);
    return lua::pushboolean(L, std::filesystem::is_directory(worldsDir));
}

const luaL_Reg worldlib [] = {
    {"get_list", lua::wrap<l_world_get_list>},
    {"get_total_time", lua::wrap<l_world_get_total_time>},
    {"get_day_time", lua::wrap<l_world_get_day_time>},
    {"set_day_time", lua::wrap<l_world_set_day_time>},
    {"get_seed", lua::wrap<l_world_get_seed>},
    {"exists", lua::wrap<l_world_exists>},
    {NULL, NULL}
};
