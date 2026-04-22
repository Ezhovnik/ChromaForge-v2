#pragma once

#include <string>

#include <logic/scripting/lua/lua_util.h>

extern const luaL_Reg packlib [];
extern const luaL_Reg timelib [];
extern const luaL_Reg filelib [];
extern const luaL_Reg worldlib [];
extern const luaL_Reg blocklib [];
extern const luaL_Reg itemlib [];
extern const luaL_Reg playerlib [];
extern const luaL_Reg inventorylib [];
extern const luaL_Reg guilib [];
extern const luaL_Reg hudlib [];
extern const luaL_Reg audiolib [];
extern const luaL_Reg builtinlib [];
extern const luaL_Reg jsonlib [];
extern const luaL_Reg inputlib [];
extern const luaL_Reg consolelib [];
extern const luaL_Reg tomllib [];
extern const luaL_Reg mat4lib [];
extern const luaL_Reg entitylib [];
extern const luaL_Reg vec2lib []; // vecn.cpp
extern const luaL_Reg vec3lib []; // vecn.cpp
extern const luaL_Reg vec4lib []; // vecn.cpp
extern const luaL_Reg cameralib [];
extern const luaL_Reg quatlib [];

extern const luaL_Reg skeletonlib [];
extern const luaL_Reg rigidbodylib [];
extern const luaL_Reg transformlib [];

extern int l_print(lua::State* L);

namespace lua {
    inline uint check_argc(lua::State* L, int a) {
        int argc = lua::gettop(L);
        if (argc == a) {
            return static_cast<uint>(argc);
        } else {
            log_error("Invalid number of arguments (" + std::to_string(a) + " expected)");
            throw std::runtime_error("Invalid number of arguments (" + std::to_string(a) + " expected)");
        }
    }

    [[nodiscard]]
    inline uint check_argc(lua::State* L, int a, int b) {
        int argc = lua::gettop(L);
        if (argc == a || argc == b) {
            return static_cast<uint>(argc);
        } else {
            log_error("Invalid number of arguments (" + std::to_string(a) + " or " + std::to_string(b) + " expected)");
            throw std::runtime_error("Invalid number of arguments (" + std::to_string(a) + " or " + std::to_string(b) + " expected)");
        }
    }
}

void initialize_libs_extends(lua::State* L);
