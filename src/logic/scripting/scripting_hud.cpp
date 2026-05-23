#include <logic/scripting/scripting_hud.h>

#include <logic/scripting/lua/lua_engine.h>
#include <logic/scripting/lua/libs/api_lua.h>
#include <engine.h>
#include <debug/Logger.h>
#include <files/files.h>
#include <frontend/hud.h>
#include <objects/Player.h>
#include <graphics/render/WorldRenderer.h>

Hud* scripting::hud = nullptr;
WorldRenderer* scripting::renderer = nullptr;

void scripting::on_frontend_init(Hud* hud, WorldRenderer* renderer) {
    scripting::hud = hud;
    scripting::renderer = renderer;

    auto L = lua::get_main_state();

    lua::openlib(L, "hud", hudlib);
    lua::openlib(L, "particles", particleslib);

    if (lua::getglobal(L, "__vc_create_hud_rules")) {
        lua::call_nothrow(L, 0, 0);
    }

    for (auto& pack : engine->getContentPacks()) {
        lua::emit_event(lua::get_main_state(), pack.id + ":.hudopen", 
        [&] (lua::State* L) {
            return lua::pushinteger(L, hud->getPlayer()->getId());        
        });
    }
}

void scripting::on_frontend_close() {
    for (auto& pack : engine->getContentPacks()) {
        lua::emit_event(lua::get_main_state(), pack.id + ":.hudclose", 
        [&] (lua::State* L) {
            return lua::pushinteger(L, hud->getPlayer()->getId());            
        });
    }
    scripting::hud = nullptr;
}

void scripting::on_frontend_render() {
    for (auto& pack : engine->getContentPacks()) {
        lua::emit_event(lua::get_main_state(), pack.id + ":.hudrender", 
        [&] (lua::State* L) {
            return 0;            
        });
    }
}

void scripting::load_hud_script(
    const scriptenv& senv,
    const std::string& packid,
    const std::filesystem::path& file
) {
    int env = *senv;
    std::string src = files::read_string(file);
    LOG_DEBUG("Loading script {}", file.u8string());

    lua::execute(lua::get_main_state(), env, src, file.u8string());

    register_event(env, "init", packid + ":.init");
    register_event(env, "on_hud_open", packid + ":.hudopen");
    register_event(env, "on_hud_render", packid + ":.hudrender");
    register_event(env, "on_hud_close", packid + ".:hudclose");

    LOG_DEBUG("Script {} successfully loaded", file.u8string());
}
