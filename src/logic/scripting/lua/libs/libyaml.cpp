#include <logic/scripting/lua/libs/api_lua.h>

#include <coders/yaml.h>
#include <coders/commons.h>

static int l_stringify(lua::State* L) {
    auto value = lua::tovalue(L, 1);
    auto string = yaml::stringify(value);
    return lua::pushstring(L, string);
}

static int l_parse(lua::State* L) {
    auto string = lua::require_string(L, 1);
    try {
        auto element = yaml::parse("[string]", string);
        return lua::pushvalue(L, element);
    } catch (const parsing_error& err) {
        throw err.toRuntimeError();
    }
}

const luaL_Reg yamllib[] = {
    {"tostring", lua::wrap<l_stringify>},
    {"parse", lua::wrap<l_parse>},
    {nullptr, nullptr}
};
