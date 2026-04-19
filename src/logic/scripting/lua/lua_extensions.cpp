#include "api_lua.h"
#include <debug/Logger.h>

static int l_debug_critical(lua::State* L) {
    auto text = lua::require_string(L, 1);
    LOG_CRITICAL("{}", text);
    return 0;
}

static int l_debug_error(lua::State* L) {
    auto text = lua::require_string(L, 1);
    LOG_ERROR("{}", text);
    return 0;
}

static int l_debug_warning(lua::State* L) {
    auto text = lua::require_string(L, 1);
    LOG_WARN("{}", text);
    return 0;
}

static int l_debug_info(lua::State* L) {
    auto text = lua::require_string(L, 1);
    LOG_INFO("{}", text);
    return 0;
}

static int l_debug_trace(lua::State* L) {
    auto text = lua::require_string(L, 1);
    LOG_TRACE("{}", text);
    return 0;
}

void initialize_libs_extends(lua::State* L) {
    if (lua::getglobal(L, "debug")) {
        lua::pushcfunction(L, lua::wrap<l_debug_critical>);
        lua::setfield(L, "critical");

        lua::pushcfunction(L, lua::wrap<l_debug_error>);
        lua::setfield(L, "error");

        lua::pushcfunction(L, lua::wrap<l_debug_warning>);
        lua::setfield(L, "warning");

        lua::pushcfunction(L, lua::wrap<l_debug_info>);
        lua::setfield(L, "info");

        lua::pop(L);
    }
}
