#include <vector>
#include <memory>
#include <sstream>

#include <logic/scripting/lua/libs/api_lua.h>
#include <engine/Engine.h>
#include <io/engine_paths.h>
#include <frontend/menu.h>
#include <frontend/screens/MenuScreen.h>
#include <logic/LevelController.h>
#include <logic/EngineController.h>
#include <io/settings_io.h>
#include <world/Level.h>
#include <data/setting.h>
#include <content/Content.h>
#include <world/generator/WorldGenerator.h>
#include <util/listutil.h>
#include <util/platform.h>
#include <io/io.h>
#include <graphics/core/Texture.h>
#include <assets/Assets.h>
#include <content/ContentControl.h>

static int l_get_version(lua::State* L) {
    return lua::pushvec_stack(
        L, glm::vec3(ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH)
    );
}

static int l_open_world(lua::State* L) {
    auto name = lua::require_string(L, 1);
    if (scripting::level != nullptr) {
        throw std::runtime_error("World must be closed before");
    }
    auto controller = scripting::engine->getController();
    controller->setLocalPlayer(0);
    controller->openWorld(name, false);
    return 0;
}

static int l_reopen_world(lua::State*) {
    auto controller = scripting::engine->getController();
    if (scripting::level == nullptr) {
        throw std::runtime_error("No world open");
    }
    controller->reopenWorld(scripting::level->getWorld());
    return 0;
}

static int l_open_folder(lua::State* L) {
    platform::open_folder(io::resolve(lua::require_string(L, 1)));
    return 0;
}

static int l_quit(lua::State*) {
    scripting::engine->quit();
    return 0;
}

static int l_delete_world(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto controller = scripting::engine->getController();
    controller->deleteWorld(name);
    return 0;
}

static int l_save_world(lua::State* L) {
    if (scripting::controller == nullptr) throw std::runtime_error("No world open");
    scripting::controller->saveWorld();
    return 0;
}

static int l_close_world(lua::State* L) {
    if (scripting::controller == nullptr) throw std::runtime_error("No world open");
    bool save_world = lua::toboolean(L, 1);
    if (save_world) scripting::controller->saveWorld();
    scripting::engine->onWorldClosed();
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
    int addLen = lua::objlen(L, 1);
    for (int i = 0; i < addLen; ++i) {
        lua::rawgeti(L, i + 1, 1);
        addPacks.emplace_back(lua::require_lstring(L, -1));
        lua::pop(L);
    }

    std::vector<std::string> remPacks;
    int remLen = lua::objlen(L, 2);
    for (int i = 0; i < remLen; ++i) {
        lua::rawgeti(L, i + 1, 2);
        remPacks.emplace_back(lua::require_lstring(L, -1));
        lua::pop(L);
    }
    auto controller = scripting::engine->getController();
    try {
        controller->reconfigPacks(scripting::controller, addPacks, remPacks);
    } catch (const contentpack_error& err) {
        throw std::runtime_error(
            std::string(err.what()) + " [" + err.getPackId() + " ]"
        );
    }
    return 0;
}

static int l_new_world(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto seed = lua::require_string(L, 2);
    auto generator = lua::require_string(L, 3);
    int64_t localPlayer = 0;
    if (lua::gettop(L) >= 4) {
        localPlayer = lua::tointeger(L, 4);
    }
    if (scripting::level != nullptr) {
        throw std::runtime_error("World must be closed before");
    }
    auto controller = scripting::engine->getController();
    controller->setLocalPlayer(localPlayer);
    controller->createWorld(name, seed, generator);
    return 0;
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
        lua::pushnumber(L, number->getDefault());
        lua::setfield(L, "def");
        return 1;
    }
    if (auto integer = dynamic_cast<IntegerSetting*>(setting)) {
        lua::pushinteger(L, integer->getMin());
        lua::setfield(L, "min");
        lua::pushinteger(L, integer->getMax());
        lua::setfield(L, "max");
        lua::pushinteger(L, integer->getDefault());
        lua::setfield(L, "def");
        return 1;
    }
    if (auto boolean = dynamic_cast<BoolSetting*>(setting)) {
        lua::pushboolean(L, boolean->getDefault());
        lua::setfield(L, "def");
        return 1;
    }
    if (auto string = dynamic_cast<StringSetting*>(setting)) {
        lua::pushstring(L, string->getDefault());
        lua::setfield(L, "def");
        return 1;
    }
    lua::pop(L);
    throw std::runtime_error("Unsupported setting type");
}

static int l_load_content(lua::State* L) {
    scripting::content_control->loadContent();
    return 0;
}

static int l_reset_content(lua::State* L) {
    if (scripting::level != nullptr) {
        throw std::runtime_error("World must be closed before");
    }
    scripting::content_control->resetContent();
    return 0;
}

static int l_is_content_loaded(lua::State* L) {
    return lua::pushboolean(L, scripting::content != nullptr);
}

static int l_blank(lua::State* L) {
    return 0;
}

static int l_capture_output(lua::State* L) {
    int argc = lua::gettop(L) - 1;
    if (!lua::isfunction(L, 1)) {
        throw std::runtime_error("Function expected as argument 1");
    }
    for (int i = 0; i < argc; ++i) {
        lua::pushvalue(L, i + 2);
    }
    lua::pushvalue(L, 1);

    auto prev_output = scripting::output_stream;
    auto prev_error = scripting::error_stream;

    std::stringstream captured_output;

    scripting::output_stream = &captured_output;
    scripting::error_stream = &captured_output;

    lua::call_nothrow(L, argc, 0);

    scripting::output_stream = prev_output;
    scripting::error_stream = prev_error;

    lua::pushstring(L, captured_output.str());
    return 1;
}

const luaL_Reg builtinlib [] = {
    {"blank", lua::wrap<l_blank>},
    {"get_version", lua::wrap<l_get_version>},
    {"load_content", lua::wrap<l_load_content>},
    {"reset_content", lua::wrap<l_reset_content>},
    {"is_content_loaded", lua::wrap<l_is_content_loaded>},
    {"new_world", lua::wrap<l_new_world>},
    {"open_world", lua::wrap<l_open_world>},
    {"reopen_world", lua::wrap<l_reopen_world>},
    {"save_world", lua::wrap<l_save_world>},
    {"close_world", lua::wrap<l_close_world>},
    {"delete_world", lua::wrap<l_delete_world>},
    {"reconfig_packs", lua::wrap<l_reconfig_packs>},
    {"get_setting", lua::wrap<l_get_setting>},
    {"set_setting", lua::wrap<l_set_setting>},
    {"str_setting", lua::wrap<l_str_setting>},
    {"get_setting_info", lua::wrap<l_get_setting_info>},
    {"open_folder", lua::wrap<l_open_folder>},
    {"quit", lua::wrap<l_quit>},
    {"capture_output", lua::wrap<l_capture_output>},
    {NULL, NULL}
};
