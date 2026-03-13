#include <iostream>

#include "api_lua.h"
#include "lua_commons.h"

int l_print(lua_State* L) {
    int n = lua_gettop(L);
    lua_getglobal(L, "tostring");
    for (int i = 1; i <= n; ++i) {
        lua_pushvalue(L, -1);
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        const char* s = lua_tostring(L, -1);
        if (s == NULL) {
            return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));
        }
        if (i > 1) {
            std::cout << "\t";
        }
        std::cout << s;
        lua_pop(L, 1);
    }
    std::cout << std::endl;
    return 0;
}
