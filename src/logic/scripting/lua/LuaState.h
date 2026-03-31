#ifndef LOGIC_SCRIPTING_LUA_LUASTATE_H_
#define LOGIC_SCRIPTING_LUA_LUASTATE_H_

#include <string>
#include <stdexcept>

#include "lua_commons.h"
#include "../../../data/dynamic.h"
#include "../../../delegates.h"
#include "../scripting_functional.h"

#ifndef LUAJIT_VERSION
#error LuaJIT required
#endif // LUAJIT_VERSION

namespace lua {
    class luaerror : public std::runtime_error {
    public:
        luaerror(const std::string& message);
    };

    class LuaState {
    private:
        lua_State* L;
        int nextEnvironment = 1;

        void logError(const std::string& text);

        void removeLibFuncs(const char* libname, const char* funcs[]);
        void createLibs();

        std::shared_ptr<std::string> createLambdaHandler();
    public:
        LuaState();
        ~LuaState();

        static const std::string envName(int env);
        void loadbuffer(int env, const std::string& src, const std::string& file);
        int gettop() const;

        int pushivec3(luaint x, luaint y, luaint z);
        int pushinteger(luaint x);
        int pushnumber(luanumber x);
        int pushstring(const std::string& str);
        int pushenv(int env);
        int pushvalue(int idx);
        int pushvalue(const dynamic::Value& value);
        int pushnil();
        int pushglobals();
        int pushboolean(bool x);
        int pushcfunction(lua_CFunction function);
        void pop(int n = 1);
        bool getfield(const std::string& name, int idx = -1);
        void setfield(const std::string& name, int idx = -2);

        const char* requireString(int idx);

        bool toboolean(int index);
        luaint tointeger(int index);
        luanumber tonumber(int index);
        const char* tostring(int index);
        dynamic::Value tovalue(int index);
        glm::vec2 tovec2(int index);
        glm::vec4 tocolor(int index);

        bool isstring(int idx);
        bool isfunction(int idx);

        int call(int argc, int nresults=-1);
        int callNoThrow(int argc);
        int execute(int env, const std::string& src, const std::string& file="<string>");
        int eval(int env, const std::string& src, const std::string& file="<eval>");
        void openlib(const std::string& name, const luaL_Reg* libfuncs);
        void addfunc(const std::string& name, lua_CFunction func);

        bool getglobal(const std::string& name);
        bool hasglobal(const std::string& name);
        void setglobal(const std::string& name);

        bool rename(const std::string& from, const std::string& to);
        void remove(const std::string& name);;

        int createEnvironment(int parent);
        void removeEnvironment(int id);

        bool emit_event(const std::string& name, std::function<int(lua::LuaState*)> args=[](auto*){return 0;});

        runnable createRunnable();
        scripting::common_func createLambda();

        void dumpStack();

        lua_State* getLua() const;
    };
}

#endif // LOGIC_SCRIPTING_LUA_LUASTATE_H_
