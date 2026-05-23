#include <filesystem>

#include <logic/scripting/lua/libs/api_lua.h>
#include <window/input.h>
#include <window/Events.h>
#include <frontend/screens/Screen.h>
#include <engine.h>
#include <frontend/hud.h>
#include <util/stringutil.h>
#include <graphics/ui/GUI.h>
#include <files/files.h>

namespace scripting {
    extern Hud* hud;
}

static int l_keycode(lua::State* L) {
    auto name = lua::require_string(L, 1);
    return lua::pushinteger(L, static_cast<int>(input_util::keycode_from(name)));
}

static int l_mousecode(lua::State* L) {
    auto name = lua::require_string(L, 1);
    return lua::pushinteger(L, static_cast<int>(input_util::mousecode_from(name)));
}

static int l_add_callback(lua::State* L) {
    auto bindname = lua::require_string(L, 1);
    const auto& bind = Events::bindings.find(bindname);
    if (bind == Events::bindings.end()) {
        throw std::runtime_error("Unknown binding: " + util::quote(bindname));
    }
    lua::pushvalue(L, 2);
    runnable actual_callback = lua::create_runnable(L);
    runnable callback = [=]() {
        if (!scripting::engine->getGUI()->isFocusCaught()) {
            actual_callback();
        }
    };
    if (scripting::hud) {
        scripting::hud->keepAlive(bind->second.onactived.add(callback));
    } else {
        throw std::runtime_error("'on_hud_open' is not called yet");
    }
    return 0;
}

static int l_get_mouse_pos(lua::State* L) {
    return lua::pushvec2(L, Events::cursor);
}

static int l_get_bindings(lua::State* L) {
    auto& bindings = Events::bindings;
    lua::createtable(L, bindings.size(), 0);

    int i = 0;
    for (auto& entry : bindings) {
        lua::pushstring(L, entry.first);
        lua::rawseti(L, i + 1);
        ++i;
    }
    return 1;
}

static int l_get_binding_text(lua::State* L) {
    auto bindname = lua::require_string(L, 1);
    auto index = Events::bindings.find(bindname);

    if (index == Events::bindings.end()) {
        throw std::runtime_error("Unknown binding " + util::quote(bindname));
        lua::pushstring(L, "");
    } else {
        lua::pushstring(L, index->second.text());
    }

    return 1;
}

static int l_is_active(lua::State* L) {
    auto bindname = lua::require_string(L, 1);
    const auto& bind = Events::bindings.find(bindname);
    if (bind == Events::bindings.end()) {
        throw std::runtime_error("Unknown binding " + util::quote(bindname));
    }
    return lua::pushboolean(L, bind->second.isActive());
}

static int l_is_pressed(lua::State* L) {
    std::string code = lua::require_string(L, 1);
    size_t sep = code.find(':');
    if (sep == std::string::npos) {
        throw std::runtime_error("Expected 'input_type:key' format");
    }
    auto prefix = code.substr(0, sep);
    auto name = code.substr(sep + 1);
    if (prefix == "key") {
        return lua::pushboolean(L, Events::isPressed(static_cast<int>(input_util::keycode_from(name))));
    } else if (prefix == "mouse") {
        return lua::pushboolean(L, Events::isClicked(static_cast<int>(input_util::mousecode_from(name))));
    } else {
        throw std::runtime_error("Unknown input type " + util::quote(code));
    }
}

static void resetPackBindings(std::filesystem::path& packFolder) {
    auto configFolder = packFolder/std::filesystem::path("config");
    auto bindsFile = configFolder/std::filesystem::path("bindings.toml");
    if (std::filesystem::is_regular_file(bindsFile)) {
        Events::loadBindings(
            bindsFile.u8string(),
            files::read_string(bindsFile),
            BindType::Rebind
        );
    }
}

static int l_reset_bindings(lua::State*) {
    auto resFolder = scripting::engine->getPaths()->getResourcesFolder();
    resetPackBindings(resFolder);
    for (auto& pack : scripting::engine->getContentPacks()) {
        resetPackBindings(pack.folder);
    }
    return 0;
}

static int l_set_enabled(lua::State* L) {
    std::string bindname = lua::require_string(L, 1);
    bool enable = lua::toboolean(L, 2);
    const auto& bind = Events::bindings.find(bindname);
    if (bind == Events::bindings.end()) {
        throw std::runtime_error("Unknown binding " + util::quote(bindname));
    }
    Events::bindings[bindname].enable = enable;
    return 0;
}

const luaL_Reg inputlib [] = {
    {"keycode", lua::wrap<l_keycode>},
    {"mousecode", lua::wrap<l_mousecode>},
    {"add_callback", lua::wrap<l_add_callback>},
    {"get_mouse_pos", lua::wrap<l_get_mouse_pos>},
    {"get_bindings", lua::wrap<l_get_bindings>},
    {"get_binding_text", lua::wrap<l_get_binding_text>},
    {"is_active", lua::wrap<l_is_active>},
    {"is_pressed", lua::wrap<l_is_pressed>},
    {"reset_bindings", lua::wrap<l_reset_bindings>},
    {"set_enabled", lua::wrap<l_set_enabled>},
    {NULL, NULL}
};
