#include <logic/scripting/lua/libs/api_lua.h>

#include <content/ContentControl.h>
#include <devtools/Project.h>
#include <engine/Engine.h>
#include <engine/EnginePaths.h>
#include <frontend/locale.h>
#include <graphics/ui/elements/Menu.h>
#include <graphics/ui/gui_util.h>
#include <graphics/ui/GUI.h>
#include <io/devices/MemoryDevice.h>
#include <io/io.h>
#include <io/settings_io.h>
#include <logic/EngineController.h>
#include <logic/LevelController.h>
#include <logic/scripting/scripting.h>
#include <network/Network.h>
#include <util/platform.h>
#include <util/stringutil.h>
#include <window/Window.h>
#include <world/Level.h>

namespace {
    static std::unique_ptr<Process> sub_instance = nullptr;
}

static int l_get_version(lua::State* L) {
    return lua::pushvec_stack(
        L, glm::vec3(
            ENGINE_VERSION_MAJOR,
            ENGINE_VERSION_MINOR,
            ENGINE_VERSION_PATCH
        )
    );
}

static int l_is_content_loaded(lua::State* L) {
    return lua::pushboolean(L, scripting::content != nullptr);
}

static int l_load_content(lua::State* L) {
    scripting::content_control->loadContent();
    return 0;
}

static int l_reset_content(lua::State* L) {
    if (scripting::level != nullptr) {
        throw std::runtime_error("World must be closed before");
    }
    std::vector<std::string> nonResetPacks;
    if (lua::istable(L, 1)) {
        int len = lua::objlen(L, 1);
        for (int i = 0; i < len; ++i) {
            lua::rawgeti(L, i + 1, 1);
            nonResetPacks.emplace_back(lua::require_lstring(L, -1));
            lua::pop(L);
        }
    }
    scripting::content_control->resetContent(std::move(nonResetPacks));
    return 0;
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
    auto engineController = scripting::engine->getController();
    try {
        engineController->reconfigPacks(scripting::controller, addPacks, remPacks);
    } catch (const contentpack_error& err) {
        throw std::runtime_error(
            std::string(err.what()) + " [" + err.getPackId() + " ]"
        );
    }
    return 0;
}

static int l_start_debug_instance(lua::State* L) {
    if (
        !scripting::engine->getProject().permissions.has(Permissions::DEBUGGING)
    ) {
        throw std::runtime_error("Project has no debugging permission");
    }

    const auto& params = scripting::engine->getCoreParameters();
    if (params.subProcessDepth >= MAX_SUBPROCESS_DEPTH) {
        throw std::runtime_error("Max subprocess depth exceeded");
    }

    int port = lua::tointeger(L, 1);
    if (port == 0) {
        auto network = scripting::engine->getNetwork();
        if (network == nullptr) {
            throw std::runtime_error("Project has no network permission");
        }
        port = network->findFreePort();
        if (port == -1) {
            throw std::runtime_error("Could not find free port");
        }
    }
    auto projectPath = lua::isstring(L, 2) ? lua::require_lstring(L, 2) : "";
    auto outputPath = lua::isstring(L, 3) ? lua::require_lstring(L, 3) : "";
    const auto& paths = scripting::engine->getPaths();

    std::vector<std::string> args {
        "--res", paths.getResourcesFolder().u8string(),
        "--dir", paths.getUserFilesFolder().u8string(),
        "--dbg-server",  "tcp:" + std::to_string(port),
        "--sub-depth", std::to_string(scripting::engine->getCoreParameters().subProcessDepth + 1),
    };
    if (!projectPath.empty()) {
        args.emplace_back("--project");
        args.emplace_back(io::resolve(std::string(projectPath)).string());
    }

    platform::new_engine_instance(
        std::move(args),
        outputPath.empty() ? "" : io::resolve(std::string(outputPath)),
        false
    );
    return lua::pushinteger(L, port);
}

static int l_start_background_instance(lua::State* L) {
    if (!scripting::engine->getProject().permissions.has(Permissions::SUB_INSTANCES)) {
        throw std::runtime_error("Project has no sub-instances permission");
    }
    const auto& params = scripting::engine->getCoreParameters();
    if (params.subProcessDepth >= MAX_SUBPROCESS_DEPTH) {
        throw std::runtime_error("Max subprocess depth exceeded");
    }

    auto scriptPath = lua::require_lstring(L, 1);
    io::path outputPath = lua::isstring(L, 2) ? lua::require_lstring(L, 2) : "";
    const auto& paths = scripting::engine->getPaths();

    std::vector<std::string> args {
        "--headless",
        "--res", paths.getResourcesFolder().u8string(),
        "--dir", paths.getUserFilesFolder().u8string(),
        "--script", io::resolve(scriptPath).u8string(),
        "--sub-depth", std::to_string(scripting::engine->getCoreParameters().subProcessDepth + 1),
    };
    args.emplace_back("--project");
    args.emplace_back(io::resolve(scripting::engine->getProject().path).u8string());

    ::sub_instance = platform::new_engine_instance(
        std::move(args),
        outputPath.empty() ? "" : io::resolve(outputPath),
        true 
    );
    return 0;
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

static int l_get_content_sources(lua::State* L) {
    const auto& sources = scripting::engine->getContentControl().getContentSources();
    lua::createtable(L, static_cast<int>(sources.size()), 0);
    for (size_t i = 0; i < sources.size(); ++i) {
        lua::pushlstring(L, sources[i].string());
        lua::rawseti(L, static_cast<int>(i + 1));
    }
    return 1;
}

static int l_set_content_sources(lua::State* L) {
    if (!lua::istable(L, 1)) {
        throw std::runtime_error("Table expected as argument 1");
    }
    int len = lua::objlen(L, 1);
    std::vector<io::path> sources;
    for (int i = 0; i < len; ++i) {
        lua::rawgeti(L, i + 1);
        sources.emplace_back(std::string(lua::require_lstring(L, -1)));
        lua::pop(L);
    }
    scripting::engine->getContentControl().setContentSources(std::move(sources));
    return 0;
}

static int l_reset_content_sources(lua::State* L) {
    scripting::engine->getContentControl().resetContentSources();
    return 0;
}

static int l_set_title(lua::State* L) {
    auto title = lua::require_string(L, 1);
    scripting::engine->getWindow().setTitle(title);
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

static int l_open_folder(lua::State* L) {
    platform::open_folder(io::resolve(lua::require_string(L, 1)));
    return 0;
}

static int l_open_url(lua::State* L) {
    auto url = lua::require_string(L, 1);

    std::wstring msg = 
        langs::get(L"Are you sure you want to open the link:") +
        L"\n" + util::str2wstr_utf8(url) +
        std::wstring(L"?");

    auto menu = scripting::engine->getGUI().getMenu();

    guiutil::confirm(*scripting::engine, msg, [url, menu]() {
        platform::open_url(url);
        if (!menu->back()) {
            menu->reset();
        }
    });
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

static int l_save_world(lua::State* L) {
    if (scripting::controller == nullptr) {
        throw std::runtime_error("No world open");
    }
    scripting::controller->saveWorld();
    return 0;
}

static int l_close_world(lua::State* L) {
    if (scripting::controller == nullptr) {
        throw std::runtime_error("No world open");
    }
    scripting::controller->processBeforeQuit();
    bool save_world = lua::toboolean(L, 1);
    if (save_world) {
        scripting::controller->saveWorld();
    }
    scripting::engine->onWorldClosed();
    return 0;
}

static int l_delete_world(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto controller = scripting::engine->getController();
    controller->deleteWorld(name);
    return 0;
}

static int l_quit(lua::State*) {
    scripting::engine->quit();
    return 0;
}

const luaL_Reg applib[] = {
    {"get_version", lua::wrap<l_get_version>},
    {"is_content_loaded", lua::wrap<l_is_content_loaded>},
    {"load_content", lua::wrap<l_load_content>},
    {"reset_content", lua::wrap<l_reset_content>},
    {"reconfig_packs", lua::wrap<l_reconfig_packs>},
    {"start_debug_instance", lua::wrap<l_start_debug_instance>},
    {"start_background_instance", lua::wrap<l_start_background_instance>},
    {"focus", lua::wrap<l_focus>},
    {"create_memory_device", lua::wrap<l_create_memory_device>},
    {"get_content_sources", lua::wrap<l_get_content_sources>},
    {"set_content_sources", lua::wrap<l_set_content_sources>},
    {"reset_content_sources", lua::wrap<l_reset_content_sources>},
    {"set_title", lua::wrap<l_set_title>},
    {"open_folder", lua::wrap<l_open_folder>},
    {"open_url", lua::wrap<l_open_url>},
    {"get_setting", lua::wrap<l_get_setting>},
    {"set_setting", lua::wrap<l_set_setting>},
    {"str_setting", lua::wrap<l_str_setting>},
    {"get_setting_info", lua::wrap<l_get_setting_info>},
    {"new_world", lua::wrap<l_new_world>},
    {"open_world", lua::wrap<l_open_world>},
    {"reopen_world", lua::wrap<l_reopen_world>},
    {"save_world", lua::wrap<l_save_world>},
    {"close_world", lua::wrap<l_close_world>},
    {"delete_world", lua::wrap<l_delete_world>},
    {"quit", lua::wrap<l_quit>},
    // For other functions see libbuiltin.cpp and res/scripts/stdlib.lua
    {nullptr, nullptr}
};
