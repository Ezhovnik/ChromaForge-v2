#include <logic/scripting/lua/libs/api_lua.h>

#include <engine/Engine.h>
#include <frontend/screens/MenuScreen.h>
#include <frontend/Panorama.h>

static MenuScreen* require_menu_screen() {
    auto screen = scripting::engine->getScreen();
    if (screen == nullptr) {
        throw std::runtime_error("No screen is currently active");
    }
    auto* menu = dynamic_cast<MenuScreen*>(screen.get());
    if (menu == nullptr) {
        throw std::runtime_error(
            "Panorama is not supported by the current screen"
        );
    }
    return menu;
}

static Panorama* require_panorama() {
    auto* menu = require_menu_screen();
    auto* panorama = menu->getPanorama();
    if (panorama == nullptr) {
        throw std::runtime_error("Panorama is not loaded");
    }
    return panorama;
}

static int l_create(lua::State* L) {
    if (scripting::engine->isHeadless()) return 0;
    auto screen = scripting::engine->getScreen();
    if (screen == nullptr) return 0;
    if (auto* menu = dynamic_cast<MenuScreen*>(screen.get())) {
        return lua::pushboolean(L, menu->createPanorama());
    }
    return lua::pushboolean(L, false);
}

static int l_destruct(lua::State* L) {
    if (scripting::engine->isHeadless()) return 0;
    auto screen = scripting::engine->getScreen();
    if (screen == nullptr) return 0;
    if (auto* menu = dynamic_cast<MenuScreen*>(screen.get())) {
        menu->destroyPanorama();
    }
    return 0;
}

static int l_is_valid(lua::State* L) {
    if (scripting::engine->isHeadless()) {
        return lua::pushboolean(L, false);
    }
    auto screen = scripting::engine->getScreen();
    auto* menu = screen ? dynamic_cast<MenuScreen*>(screen.get()) : nullptr;
    return lua::pushboolean(L, menu && menu->getPanorama() != nullptr);
}

static int l_get_rotation_speed(lua::State* L) {
    return lua::pushnumber(L, require_panorama()->getRotationSpeed());
}

static int l_set_rotation_speed(lua::State* L) {
    require_panorama()->setRotationSpeed(static_cast<float>(lua::tonumber(L, 1)));
    return 0;
}

static int l_get_rotation(lua::State* L) {
    return lua::pushnumber(L, require_panorama()->getRotation());
}

static int l_set_rotation(lua::State* L) {
    require_panorama()->setRotation(static_cast<float>(lua::tonumber(L, 1)));
    return 0;
}

static int l_set_textures(lua::State* L) {
    std::array<std::string, 6> faces;
    for (int i = 0; i < 6; ++i) {
        faces[i] = std::string(lua::tostring(L, i + 1));
    }
    return lua::pushboolean(L, require_panorama()->setTextures(faces));
}

const luaL_Reg panoramalib[] = {
    {"create", lua::wrap<l_create>},
    {"destruct", lua::wrap<l_destruct>},
    {"is_valid", lua::wrap<l_is_valid>},
    {"set_textures", lua::wrap<l_set_textures>},
    {"get_rotation_speed", lua::wrap<l_get_rotation_speed>},
    {"set_rotation_speed", lua::wrap<l_set_rotation_speed>},
    {"get_rotation", lua::wrap<l_get_rotation>},
    {"set_rotation", lua::wrap<l_set_rotation>},
    {nullptr, nullptr}
};