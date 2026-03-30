#include "api_lua.h"
#include "lua_commons.h"
#include "LuaState.h"
#include "../scripting.h"
#include "../../CommandsInterpreter.h"
#include "../../../engine.h"
#include "../../../coders/commons.h"

namespace scripting {
    extern lua::LuaState* state;
}

using namespace scripting;

static int l_add_command(lua_State* L) {
    auto scheme = lua_tostring(L, 1);
    lua_pushvalue(L, 2);
    auto func = state->createLambda();
    try {
        engine->getCommandsInterpreter()->getRepository()->add(
            scheme, [func](auto, auto args, auto kwargs) {
                return func({args, kwargs});
            }
        );
    } catch (const parsing_error& err) {
        luaL_error(L, ("Command scheme error:\n"+err.errorLog()).c_str());
    }
    return 0;
}

static int l_execute(lua_State* L) {
    auto prompt = lua_tostring(L, 1);
    auto result = engine->getCommandsInterpreter()->execute(prompt);
    state->pushvalue(result);
    return 1;
}

const luaL_Reg consolelib [] = {
    {"add_command", lua_wrap_errors<l_add_command>},
    {"execute", lua_wrap_errors<l_execute>},
    {NULL, NULL}
};
