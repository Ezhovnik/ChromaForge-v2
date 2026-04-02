#include "scripting_hud.h"

#include "scripting.h"
#include "lua/api_lua.h"
#include "lua/LuaState.h"
#include "../../engine.h"
#include "../../debug/Logger.h"
#include "../../files/files.h"
#include "../../frontend/hud.h"
#include "../../objects/Player.h"

namespace scripting {
    extern lua::LuaState* state;
}

Hud* scripting::hud = nullptr;

void scripting::on_frontend_init(Hud* hud) {
    scripting::hud = hud;
    scripting::state->openlib("hud", hudlib);

    for (auto& pack : scripting::engine->getContentPacks()) {
        state->emit_event(pack.id + ".hudopen", [&] (lua::LuaState* state) {
            state->pushinteger(hud->getPlayer()->getId());
            return 1;            
        });
    }
}

void scripting::on_frontend_close() {
    for (auto& pack : scripting::engine->getContentPacks()) {
        state->emit_event(pack.id + ".hudclose", [&] (lua::LuaState* state) {
            state->pushinteger(hud->getPlayer()->getId());
            return 1;            
        });
    }
    scripting::hud = nullptr;
}

void scripting::load_hud_script(
    const scriptenv& senv,
    const std::string& packid,
    const std::filesystem::path& file
) {
    int env = *senv;
    std::string src = files::read_string(file);
    LOG_DEBUG("Loading script {}", file.u8string());

    state->loadbuffer(env, src, file.u8string());
    state->callNoThrow(0);

    register_event(env, "init", packid + ".init");
    register_event(env, "on_hud_open", packid + ".hudopen");
    register_event(env, "on_hud_close", packid + ".hudclose");

    LOG_DEBUG("Script {} successfully loaded", file.u8string());
}
