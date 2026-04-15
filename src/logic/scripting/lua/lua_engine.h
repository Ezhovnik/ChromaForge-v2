#ifndef LOGIC_SCRIPTING_LUA_ENGINE_H_
#define LOGIC_SCRIPTING_LUA_ENGINE_H_

#include "lua_util.h"

#include "logic/scripting/scripting_functional.h"
#include "data/dynamic.h"
#include "delegates.h"

#include <string>
#include <stdexcept>

namespace lua {
    void initialize();
    void finalize();

    bool emit_event(lua::State*, const std::string& name, std::function<int(lua::State*)> args=[](auto*){return 0;});
    lua::State* get_main_thread();
}

#endif // LOGIC_SCRIPTING_LUA_ENGINE_H_
