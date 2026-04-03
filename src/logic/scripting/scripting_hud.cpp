#include "scripting_hud.h"

#include "lua/lua_engine.h"
#include "lua/api_lua.h"
#include "../../engine.h"
#include "../../debug/Logger.h"
#include "../../files/files.h"
#include "../../frontend/hud.h"
#include "../../objects/Player.h"

Hud* scripting::hud = nullptr;

void scripting::on_frontend_init(Hud* hud) {
    scripting::hud = hud;
    lua::openlib(lua::get_main_thread(), "hud", hudlib);

    for (auto& pack : engine->getContentPacks()) {
        lua::emit_event(lua::get_main_thread(), pack.id + ".hudopen", 
        [&] (lua::State* L) {
            return lua::pushinteger(L, hud->getPlayer()->getId());        
        });
    }
}

void scripting::on_frontend_close() {
    for (auto& pack : engine->getContentPacks()) {
        lua::emit_event(lua::get_main_thread(), pack.id + ".hudclose", 
        [&] (lua::State* L) {
            return lua::pushinteger(L, hud->getPlayer()->getId());            
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

    lua::execute(lua::get_main_thread(), env, src, file.u8string());

    register_event(env, "init", packid + ".init");
    register_event(env, "on_hud_open", packid + ".hudopen");
    register_event(env, "on_hud_close", packid + ".hudclose");

    LOG_DEBUG("Script {} successfully loaded", file.u8string());
}
