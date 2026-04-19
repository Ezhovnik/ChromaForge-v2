#include <vector>
#include <memory>

#include "api_lua.h"
#include <engine.h>
#include "files/engine_paths.h"
#include "frontend/menu.h"
#include <window/Window.h>
#include "frontend/screens/MenuScreen.h"
#include "logic/LevelController.h"
#include <window/Events.h>
#include <world/WorldGenerators.h>
#include "logic/EngineController.h"
#include "files/settings_io.h"
#include <world/Level.h>
#include "data/setting.h"

static int l_open_world(lua::State* L) {
    auto name = lua::require_string(L, 1);
    scripting::engine->setScreen(nullptr);
    auto controller = scripting::engine->getController();
    controller->openWorld(name, false);
    return 0;
}

static int l_reopen_world(lua::State*) {
    auto controller = scripting::engine->getController();
    controller->reopenWorld(scripting::level->getWorld());
    return 0;
}

static int l_quit(lua::State*) {
    Window::setShouldClose(true);
    return 0;
}

static int l_delete_world(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto controller = scripting::engine->getController();
    controller->deleteWorld(name);
    return 0;
}

static int l_close_world(lua::State* L) {
    if (scripting::controller == nullptr) throw std::runtime_error("No world open");
    bool save_world = lua::toboolean(L, 1);
    if (save_world) scripting::controller->saveWorld();
    scripting::engine->setScreen(nullptr);
    scripting::engine->setScreen(std::make_shared<MenuScreen>(scripting::engine));
    return 0;
}

static int l_get_setting(lua::State* L) {
    auto name = lua::require_string(L, 1);
    const auto value = scripting::engine->getSettingsHandler().getValue(name);
    return lua::pushvalue(L, value);
}

static int l_set_setting(lua::State* L) {
    auto name = lua::require_string(L, 1);
    const auto value = lua::tovalue(L, 2);
    scripting::engine->getSettingsHandler().setValue(name, value);
    return 0;
}

static int l_str_setting(lua::State* L) {
    auto name = lua::require_string(L, 1);
    const auto string = scripting::engine->getSettingsHandler().toString(name);
    return lua::pushstring(L, string);
}

static int l_reconfig_packs(lua::State* L) {
    if (!lua::istable(L, 1)) {
        throw std::runtime_error("Strings array expected as the first argument");
    }
    if (!lua::istable(L, 2)) {
        throw std::runtime_error("Strings array expected as the second argument");
    }

    std::vector<std::string> addPacks;
    if (!lua::istable(L, 1)) {
        throw std::runtime_error("An array expected as argument 1");
    }
    int addLen = lua::objlen(L, 1);
    for (int i = 0; i < addLen; ++i) {
        lua::rawgeti(L, i + 1, 1);
        addPacks.emplace_back(lua::tostring(L, -1));
        lua::pop(L);
    }

    std::vector<std::string> remPacks;
    if (!lua::istable(L, 2)) {
        throw std::runtime_error("An array expected as argument 2");
    }
    int remLen = lua::objlen(L, 2);
    for (int i = 0; i < remLen; ++i) {
        lua::rawgeti(L, i + 1, 2);
        remPacks.emplace_back(lua::tostring(L, -1));
        lua::pop(L);
    }
    auto controller = scripting::engine->getController();
    controller->reconfigPacks(scripting::controller, addPacks, remPacks);
    return 0;
}

static int l_new_world(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto seed = lua::require_string(L, 2);
    auto generator = lua::require_string(L, 3);
    auto controller = scripting::engine->getController();
    controller->createWorld(name, seed, generator);
    return 0;
}

static int l_get_default_generator(lua::State* L) {
    return lua::pushstring(L, WorldGenerators::getDefaultGeneratorID());
}

static int l_get_generators(lua::State* L) {
    const auto& generators = WorldGenerators::getGeneratorsIDs();
    lua::createtable(L, generators.size(), 0);

    int i = 0;
    for (auto& id : generators) {
        lua::pushstring(L, id);
        lua::rawseti(L, i + 1);
        ++i;
    }
    return 1;
}

static int l_get_setting_info(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto setting = scripting::engine->getSettingsHandler().getSetting(name);
    lua::createtable(L, 0, 1);
    if (auto number = dynamic_cast<NumberSetting*>(setting)) {
        lua::pushnumber(L, number->getMin());
        lua::setfield(L, "min");
        lua::pushnumber(L, number->getMax());
        lua::setfield(L, "max");
        return 1;
    }
    if (auto integer = dynamic_cast<IntegerSetting*>(setting)) {
        lua::pushinteger(L, integer->getMin());
        lua::setfield(L, "min");
        lua::pushinteger(L, integer->getMax());
        lua::setfield(L, "max");
        return 1;
    }
    lua::pop(L);
    throw std::runtime_error("Unsupported setting type");
}

const luaL_Reg builtinlib [] = {
    {"new_world", lua::wrap<l_new_world>},
    {"open_world", lua::wrap<l_open_world>},
    {"reopen_world", lua::wrap<l_reopen_world>},
    {"close_world", lua::wrap<l_close_world>},
    {"delete_world", lua::wrap<l_delete_world>},
    {"reconfig_packs", lua::wrap<l_reconfig_packs>},
    {"get_setting", lua::wrap<l_get_setting>},
    {"set_setting", lua::wrap<l_set_setting>},
    {"str_setting", lua::wrap<l_str_setting>},
    {"get_setting_info", lua::wrap<l_get_setting_info>},
    {"quit", lua::wrap<l_quit>},
    {"get_default_generator", lua::wrap<l_get_default_generator>},
    {"get_generators", lua::wrap<l_get_generators>},
    {NULL, NULL}
};
