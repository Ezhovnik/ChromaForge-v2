#pragma once

#include <logic/scripting/lua/lua_util.h>

#include <logic/scripting/scripting_functional.h>
#include <delegates.h>

#include <string>
#include <stdexcept>

namespace lua {
    enum class StateType {
        Base,
        Generator
    };

    void initialize();
    void finalize();

    bool emit_event(State*, const std::string& name, std::function<int(State*)> args=[](auto*){return 0;});
    State* get_main_state();
    State* create_state(StateType stateType);
    [[nodiscard]] scriptenv create_environment(State* L);

    void init_state(State* L, StateType stateType);
}
