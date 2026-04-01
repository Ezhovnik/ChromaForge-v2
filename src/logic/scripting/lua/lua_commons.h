#ifndef LOGIC_SCRIPTING_LUA_LUA_COMMONS_H_
#define LOGIC_SCRIPTING_LUA_LUA_COMMONS_H_

#ifdef __linux__ 
#include <luajit-2.1/luaconf.h>
#include <luajit-2.1/lua.hpp>
#else
#include <lua.hpp>
#endif
#include <exception>
#include <string>

#ifndef LUAJIT_VERSION
#error LuaJIT required
#endif

template <lua_CFunction func> int lua_wrap_errors(lua_State *L) {
    int result = 0;
    try {
        result = func(L);
    } catch (std::exception &e) {
        luaL_error(L, e.what());
    } catch (...) {
        throw;
    }
    return result;
}

#endif // LOGIC_SCRIPTING_LUA_LUA_COMMONS_H_
