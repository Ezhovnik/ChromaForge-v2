#include <logic/scripting/scripting_hud.h>

#include <logic/scripting/lua/lua_engine.h>
#include <logic/scripting/lua/libs/api_lua.h>
#include <engine/Engine.h>
#include <debug/Logger.h>
#include <io/io.h>
#include <frontend/hud.h>
#include <objects/Player.h>
#include <graphics/render/WorldRenderer.h>
#include <frontend/UIDocument.h>

Hud* scripting::hud = nullptr;
WorldRenderer* scripting::renderer = nullptr;

static void load_script(const std::string& name) {
    auto file = io::path("res:scripts") / name;
    std::string src = io::read_string(file);
    LOG_INFO("Loading script {}", file.string());

    lua::execute(lua::get_main_state(), 0, src, file.string());
}

void scripting::on_frontend_init(Hud* hud, WorldRenderer* renderer) {
    scripting::hud = hud;
    scripting::renderer = renderer;

    auto L = lua::get_main_state();

    lua::openlib(L, "hud", hudlib);
    lua::openlib(L, "gfx", "blockwraps", blockwrapslib);
    lua::openlib(L, "gfx", "particles", particleslib);
    lua::openlib(L, "gfx", "text3d", text3dlib);

    load_script("hud_classes.lua");

    if (lua::getglobal(L, "__chroma_on_hud_open")) {
        lua::call_nothrow(L, 0, 0);
    }

    for (auto& pack : engine->getAllContentPacks()) {
        lua::emit_event(lua::get_main_state(), pack.id + ":.hudopen", 
        [&] (lua::State* L) {
            return lua::pushinteger(L, hud->getPlayer()->getId());        
        });
    }
}

void scripting::on_frontend_close() {
    auto L = lua::get_main_state();
    for (auto& pack : engine->getAllContentPacks()) {
        lua::emit_event(L, pack.id + ":.hudclose", 
        [&] (lua::State* L) {
            return lua::pushinteger(L, hud->getPlayer()->getId());            
        });
    }
    lua::pushnil(L);
    lua::setglobal(L, "hud");
    lua::pushnil(L);
    lua::setglobal(L, "gfx");

    scripting::renderer = nullptr;
    scripting::hud = nullptr;
}

void scripting::on_frontend_render() {
    for (auto& pack : engine->getAllContentPacks()) {
        lua::emit_event(lua::get_main_state(), pack.id + ":.hudrender", 
        [] (lua::State* L) {
            return 0;            
        });
    }
}

void scripting::load_hud_script(
    const scriptenv& senv,
    const std::string& packid,
    const io::path& file,
    const std::string& fileName
) {
    int env = *senv;
    std::string src = io::read_string(file);
    LOG_DEBUG("Loading script {}", file.string());

    lua::execute(lua::get_main_state(), env, src, fileName);

    register_event(env, "init", packid + ":.init");
    register_event(env, "on_hud_open", packid + ":.hudopen");
    register_event(env, "on_hud_render", packid + ":.hudrender");
    register_event(env, "on_hud_close", packid + ".:hudclose");

    LOG_DEBUG("Script {} successfully loaded", file.string());
}

gui::PageLoaderFunc scripting::create_page_loader() {
    auto L = lua::get_main_state();
    if (lua::getglobal(L, "__chroma_page_loader")) {
        auto func = lua::create_lambda(L);
        return [func](const std::string& name) -> std::shared_ptr<gui::UINode> {
            auto docname = func({name}).asString();
            return engine->getAssets()->require<UIDocument>(docname).getRoot();
        };
    }
    return nullptr;
}
