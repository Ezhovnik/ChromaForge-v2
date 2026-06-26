#include <iostream>

#include <logic/scripting/lua/libs/api_lua.h>

int l_print(lua::State* L) {
    int n = lua::gettop(L);
    lua::getglobal(L, "tostring");
    for (int i = 1; i <= n; ++i) {
        lua::pushvalue(L, -1);
        lua::pushvalue(L, i);
        lua::call(L, 1, 1);
        const char* s = lua::tostring(L, -1);
        if (s == NULL) {
            return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));
        }
        if (i > 1) {
            *scripting::output_stream << "\t";
        }
        *scripting::output_stream << s;
        lua::pop(L);
    }
    *scripting::output_stream << std::endl;
    return 0;
}
