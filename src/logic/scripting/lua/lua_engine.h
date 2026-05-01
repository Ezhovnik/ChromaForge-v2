#pragma once

#include <logic/scripting/lua/lua_util.h>

#include <logic/scripting/scripting_functional.h>
#include <delegates.h>

#include <string>
#include <stdexcept>

namespace lua {
    void initialize();
    void finalize();

    bool emit_event(lua::State*, const std::string& name, std::function<int(lua::State*)> args=[](auto*){return 0;});
    lua::State* get_main_thread();
}
