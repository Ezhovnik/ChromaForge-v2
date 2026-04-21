#ifndef LOGIC_SCRIPTING_LUA_LIBENTITY_H_
#define LOGIC_SCRIPTING_LUA_LIBENTITY_H_

#include <optional>

#include <logic/scripting/lua/api_lua.h>
#include <logic/LevelController.h>
#include <frontend/hud.h>
#include <world/Level.h>
#include <objects/Entities.h>

namespace scripting {
    extern Hud* hud;
}

inline std::optional<Entt_Entity> get_entity(lua::State* L, int idx) {
    auto id = lua::tointeger(L, idx);
    auto level = scripting::controller->getLevel();
    return level->entities->get(id);
}

#endif // LOGIC_SCRIPTING_LUA_LIBENTITY_H_
