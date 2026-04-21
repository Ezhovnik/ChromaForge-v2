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

#include <delegates.h>
#include <logic/scripting/scripting.h>

namespace lua {
    class luaerror : public std::runtime_error {
    public:
        luaerror(const std::string& message);
    };

    void log_error(const std::string& text);

    using State = lua_State;
    using Number = lua_Number;
    using Integer = lua_Integer;
}

#endif // LOGIC_SCRIPTING_LUA_LUA_COMMONS_H_
