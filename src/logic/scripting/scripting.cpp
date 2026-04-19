#include "scripting.h"

#include <stdexcept>

#include "files/engine_paths.h"
#include "files/files.h"
#include <util/timeutil.h>
#include <world/Level.h>
#include <voxels/Block.h>
#include <debug/Logger.h>
#include "items/Item.h"
#include "logic/BlocksController.h"
#include <engine.h>
#include "content/ContentPack.h"
#include "lua/lua_engine.h"
#include <util/stringutil.h>
#include "frontend/UIDocument.h"
#include "items/Inventory.h"
#include "objects/Player.h"
#include "logic/LevelController.h"
#include "objects/Entity.h"
#include "objects/Entities.h"
#include "content/Content.h"

static inline const std::string STDCOMP = "stdcomp";

Engine* scripting::engine = nullptr;
Level* scripting::level = nullptr;
const Content* scripting::content = nullptr;
BlocksController* scripting::blocks = nullptr;
LevelController* scripting::controller = nullptr;
const ContentIndices* scripting::indices = nullptr;

static void load_script(const std::filesystem::path& name, bool throwable) {
    auto paths = scripting::engine->getPaths();
    std::filesystem::path file = paths->getResources()/std::filesystem::path("scripts")/name;

    std::string src = files::read_string(file);
    auto L = lua::get_main_thread();
    lua::loadbuffer(L, 0, src, file.u8string());
    if (throwable) {
        lua::call(L, 0, 0);
    } else {
        lua::call_nothrow(L, 0, 0);
    }
}

void scripting::initialize(Engine* engine) {
    scripting::engine = engine;

    lua::initialize();

    load_script(std::filesystem::path("stdlib.lua"), true);
    load_script(std::filesystem::path("stdcmd.lua"), true);
    load_script(std::filesystem::path("classes.lua"), true);
}

[[nodiscard]]
scriptenv scripting::get_root_environment() {
    return std::make_shared<int>(0);
}

[[nodiscard]]
scriptenv scripting::create_pack_environment(const ContentPack& pack) {
    auto L = lua::get_main_thread();
    int id = lua::create_environment(L, 0);
    lua::pushenv(L, id);
    lua::pushvalue(L, -1);
    lua::setfield(L, "PACK_ENV");
    lua::pushstring(L, pack.id);
    lua::setfield(L, "PACK_ID");
    lua::pop(L);
    return std::shared_ptr<int>(new int(id), [=](int* id) {
        lua::removeEnvironment(L, *id);
        delete id;
    });
}

[[nodiscard]]
scriptenv scripting::create_doc_environment(const scriptenv& parent, const std::string& name) {
    auto L = lua::get_main_thread();
    int id = lua::create_environment(L, *parent);
    lua::pushenv(L, id);
    lua::pushvalue(L, -1);
    lua::setfield(L, "DOC_ENV");
    lua::pushstring(L, name);
    lua::setfield(L, "DOC_NAME");

    if (lua::getglobal(L, "Document")) {
        if (lua::getfield(L, "new")) {
            lua::pushstring(L, name);
            if (lua::call_nothrow(L, 1)) {
                lua::setfield(L, "document", -3);
            }
        }
        lua::pop(L);
    }
    lua::pop(L);
    return std::shared_ptr<int>(new int(id), [=](int* id) {
        lua::removeEnvironment(L, *id);
        delete id;
    });
}

[[nodiscard]]
static scriptenv create_component_environment(
    const scriptenv& parent,
    int entityIdx,
    const std::string& name
) {
    auto L = lua::get_main_thread();
    int id = lua::create_environment(L, *parent);

    lua::pushvalue(L, entityIdx);

    lua::pushenv(L, id);

    lua::pushvalue(L, -1);
    lua::setfield(L, "this");

    lua::pushvalue(L, -2);
    lua::setfield(L, "entity");

    lua::pop(L);
    if (lua::getfield(L, "components")) {
        lua::pushenv(L, id);
        lua::setfield(L, name);
        lua::pop(L);
    }
    lua::pop(L);

    return std::shared_ptr<int>(new int(id), [=](int* id) {
        lua::removeEnvironment(L, *id);
        delete id;
    });
}

void scripting::process_post_runnables() {
    auto L = lua::get_main_thread();
    if (lua::getglobal(L, "__process_post_runnables")) {
        lua::call_nothrow(L, 0);
    }
}

void scripting::on_world_load(LevelController* controller) {
    scripting::level = controller->getLevel();
    scripting::content = level->content;
    scripting::indices = level->content->getIndices();
    scripting::blocks = controller->getBlocksController();
    scripting::controller = controller;
    load_script("world.lua", false);

    auto L = lua::get_main_thread();
    for (auto& pack : scripting::engine->getContentPacks()) {
        lua::emit_event(L, pack.id + ".worldopen");
    }
}

void scripting::on_world_spark() {
    auto L = lua::get_main_thread();
    for (auto& pack : scripting::engine->getContentPacks()) {
        lua::emit_event(L, pack.id + ".worldspark");
    }
}

void scripting::on_world_save() {
    auto L = lua::get_main_thread();
    for (auto& pack : scripting::engine->getContentPacks()) {
        lua::emit_event(L, pack.id + ".worldsave");
    }
}

void scripting::on_world_quit() {
    auto L = lua::get_main_thread();
    for (auto& pack : scripting::engine->getContentPacks()) {
        lua::emit_event(L, pack.id + ".worldquit");
    }

    lua::getglobal(L, "pack");
    for (auto& pack : scripting::engine->getContentPacks()) {
        lua::getfield(L, "unload");
        lua::pushstring(L, pack.id);
        lua::call_nothrow(L, 1);   
    }
    lua::pop(L);
    
    if (lua::getglobal(L, "__scripts_cleanup")) {
        lua::call_nothrow(L, 0);
    }
    scripting::level = nullptr;
    scripting::content = nullptr;
    scripting::indices = nullptr;
    scripting::blocks = nullptr;
    scripting::controller = nullptr;
}

void scripting::on_blocks_spark(const Block& block, int tps) {
    std::string name = block.name + ".blocksspark";
    lua::emit_event(lua::get_main_thread(), name, [tps] (auto L) {
        return lua::pushinteger(L, tps);
    });
}

void scripting::update_block(const Block& block, int x, int y, int z) {
    std::string name = block.name + ".update";
    lua::emit_event(lua::get_main_thread(), name, [x, y, z] (auto L) {
        return lua::pushivec3_stack(L, x, y, z);
    });
}

void scripting::random_update_block(const Block& block, int x, int y, int z) {
    std::string name = block.name + ".randupdate";
    lua::emit_event(lua::get_main_thread(), name, [x, y, z] (auto L) {
        return lua::pushivec3_stack(L, x, y, z);
    });
}

void scripting::on_block_placed(Player* player, const Block& block, int x, int y, int z) {
    std::string name = block.name + ".placed";
    lua::emit_event(lua::get_main_thread(), name, [x, y, z, player] (auto L) {
        lua::pushivec3_stack(L, x, y, z);
        lua::pushinteger(L, player ? player->getId() : -1);
        return 4;
    });

    auto world_event_args = [&](lua::State* L) {
        lua::pushinteger(L, block.rt.id);
        lua::pushivec3_stack(L, x, y, z);
        lua::pushinteger(L, player ? player->getId() : -1);
        return 5;
    };
    for (auto& [packid, pack] : content->getPacks()) {
        if (pack->worldfuncsset.onblockplaced) {
            lua::emit_event(lua::get_main_thread(), packid + ".blockplaced", world_event_args);
        }
    }
}

void scripting::on_block_broken(Player* player, const Block& block, int x, int y, int z) {
    if (block.rt.funcsset.onbroken) {
        std::string name = block.name + ".broken";
        lua::emit_event(lua::get_main_thread(), name, [x, y, z, player] (auto L) {
            lua::pushivec3_stack(L, x, y, z);
            lua::pushinteger(L, player ? player->getId() : -1);
            return 4;
        });
    }
    auto world_event_args = [&](lua::State* L) {
        lua::pushinteger(L, block.rt.id);
        lua::pushivec3_stack(L, x, y, z);
        lua::pushinteger(L, player ? player->getId() : -1);
        return 5;
    };
    for (auto& [packid, pack] : content->getPacks()) {
        if (pack->worldfuncsset.onblockbroken) {
            lua::emit_event(lua::get_main_thread(), packid + ".blockbroken", world_event_args);
        }
    }
}

bool scripting::on_block_interact(Player* player, const Block& block, glm::ivec3 pos) {
    std::string name = block.name + ".interact";
    return lua::emit_event(lua::get_main_thread(), name, [pos, player] (auto L) {
        lua::pushivec3_stack(L, pos.x, pos.y, pos.z);
        lua::pushinteger(L, player->getId());
        return 4;
    });
}

bool scripting::on_item_use(Player* player, const Item& item) {
    std::string name = item.name + ".use";
    return lua::emit_event(lua::get_main_thread(), name, [player] (lua::State* L) {
        return lua::pushinteger(L, player->getId());
    });
}

bool scripting::on_item_use_on_block(Player* player, const Item& item, glm::ivec3 ipos, glm::ivec3 normal) {
    std::string name = item.name + ".useon";
    return lua::emit_event(lua::get_main_thread(), name, [ipos, normal, player] (auto L) {
        lua::pushivec3_stack(L, ipos.x, ipos.y, ipos.z);
        lua::pushinteger(L, player->getId());
        lua::pushivec(L, normal);
        return 5;
    });
}

bool scripting::on_item_break_block(Player* player, const Item& item, int x, int y, int z) {
    std::string name = item.name + ".blockbreakby";
    return lua::emit_event(lua::get_main_thread(), name, [x, y, z, player] (auto L) {
        lua::pushivec3_stack(L, x, y, z);
        lua::pushinteger(L, player->getId());
        return 4;
    });
}

dynamic::Value scripting::get_component_value(const scriptenv& env, const std::string& name) {
    auto L = lua::get_main_thread();
    lua::pushenv(L, *env);
    if (lua::getfield(L, name)) {
        return lua::tovalue(L, -1);
    }
    return dynamic::NONE;
}

void scripting::on_entity_spawn(
    const Entity&,
    entityid_t eid,
    const std::vector<std::unique_ptr<UserComponent>>& components,
    dynamic::Map_sptr args,
    dynamic::Map_sptr saved
) {
    auto L = lua::get_main_thread();
    lua::requireglobal(L, STDCOMP);
    if (lua::getfield(L, "new_Entity")) {
        lua::pushinteger(L, eid);
        lua::call(L, 1);
    }

    if (components.size() > 1) {
        for (size_t i = 0; i < components.size() - 1; ++i) {
            lua::pushvalue(L, -1);
        }
    }

    for (auto& component : components) {
        auto compenv = create_component_environment(
            get_root_environment(), -1, component->name
        );
        lua::get_from(L, lua::CHUNKS_TABLE, component->name, true);
        lua::pushenv(L, *compenv);
        if (args != nullptr) {
            std::string compfieldname = component->name;
            util::replaceAll(compfieldname, ":", "__");
            if (auto datamap = args->map(compfieldname)) {
                lua::pushvalue(L, datamap);
            } else {
                lua::createtable(L, 0, 0);
            }
        } else {
            lua::createtable(L, 0, 0);
        }
        lua::setfield(L, "ARGS");

        if (saved == nullptr) {
            lua::createtable(L, 0, 0);
        } else {
            if (auto datamap = saved->map(component->name)) {
                lua::pushvalue(L, datamap);
            } else {
                lua::createtable(L, 0, 0);
            }
        }
        lua::setfield(L, "SAVED_DATA");
        lua::setfenv(L);
        lua::call_nothrow(L, 0, 0);

        lua::pushenv(L, *compenv);
        auto& funcsset = component->funcsset;
        funcsset.on_grounded = lua::hasfield(L, "on_grounded");
        funcsset.on_fall = lua::hasfield(L, "on_fall");
        funcsset.on_despawn = lua::hasfield(L, "on_despawn");
        funcsset.on_save = lua::hasfield(L, "on_save");
        funcsset.on_sensor_enter = lua::hasfield(L, "on_sensor_enter");
        funcsset.on_sensor_exit = lua::hasfield(L, "on_sensor_exit");
        funcsset.on_aim_on = lua::hasfield(L, "on_aim_on");
        funcsset.on_aim_off = lua::hasfield(L, "on_aim_off");
        funcsset.on_attacked = lua::hasfield(L, "on_attacked");
        funcsset.on_used = lua::hasfield(L, "on_used");
        lua::pop(L, 2);

        component->env = compenv;
    }
}

static void process_entity_callback(
    const scriptenv& env, 
    const std::string& name, 
    std::function<int(lua::State*)> args
) {
    auto L = lua::get_main_thread();
    lua::pushenv(L, *env);
    if (lua::getfield(L, name)) {
        if (args) {
            lua::call_nothrow(L, args(L), 0);
        } else {
            lua::call_nothrow(L, 0, 0);
        }
    }
    lua::pop(L);
}

static void process_entity_callback(
    const Entt_Entity& entity,
    const std::string& name,
    bool entity_funcs_set::*flag,
    std::function<int(lua::State*)> args
) {
    const auto& script = entity.getScripting();
    for (auto& component : script.components) {
        if (component->funcsset.*flag) {
            process_entity_callback(component->env, name, args);
        }
    }
}

void scripting::on_entity_despawn(const Entt_Entity& entity) {
    process_entity_callback(entity, "on_despawn", &entity_funcs_set::on_despawn, nullptr);

    auto L = lua::get_main_thread();
    lua::get_from(L, "stdcomp", "remove_Entity", true);
    lua::pushinteger(L, entity.getUID());
    lua::call(L, 1, 0);
}

void scripting::on_entity_grounded(const Entt_Entity& entity, float force) {
    process_entity_callback(entity, "on_grounded", &entity_funcs_set::on_grounded, [force](auto L){
        return lua::pushnumber(L, force);
    });
}

void scripting::on_entity_fall(const Entt_Entity& entity) {
    process_entity_callback(entity, "on_fall", &entity_funcs_set::on_fall, nullptr);
}

void scripting::on_entity_save(const Entt_Entity& entity) {
    process_entity_callback(entity, "on_save", &entity_funcs_set::on_save, nullptr);
}

void scripting::on_sensor_enter(const Entt_Entity& entity, size_t index, entityid_t oid) {
    process_entity_callback(entity, "on_sensor_enter", &entity_funcs_set::on_sensor_enter, [index, oid](auto L) {
        lua::pushinteger(L, index);
        lua::pushinteger(L, oid);
        return 2;
    });
}

void scripting::on_sensor_exit(const Entt_Entity& entity, size_t index, entityid_t oid) {
    process_entity_callback(entity, "on_sensor_exit", &entity_funcs_set::on_sensor_exit, [index, oid](auto L) {
        lua::pushinteger(L, index);
        lua::pushinteger(L, oid);
        return 2;
    });
}

void scripting::on_aim_on(const Entt_Entity& entity, Player* player) {
    process_entity_callback(entity, "on_aim_on", 
            &entity_funcs_set::on_aim_on, [player](auto L) {
        return lua::pushinteger(L, player->getId());
    });
}

void scripting::on_aim_off(const Entt_Entity& entity, Player* player) {
    process_entity_callback(entity, "on_aim_off", 
            &entity_funcs_set::on_aim_off, [player](auto L) {
        return lua::pushinteger(L, player->getId());
    });
}

void scripting::on_attacked(const Entt_Entity& entity, Player* player, entityid_t attacker) {
    process_entity_callback(entity, "on_attacked", &entity_funcs_set::on_attacked, [player, attacker](auto L) {
        lua::pushinteger(L, attacker);
        lua::pushinteger(L, player->getId());
        return 2;
    });
}

void scripting::on_entity_used(const Entt_Entity& entity, Player* player) {
    process_entity_callback(entity, "on_used", &entity_funcs_set::on_used, [player](auto L) {
        return lua::pushinteger(L, player->getId());
    });
}

void scripting::on_entities_update(int sps, int parts, int part) {
    auto L = lua::get_main_thread();
    lua::get_from(L, STDCOMP, "update", true);
    lua::pushinteger(L, sps);
    lua::pushinteger(L, parts);
    lua::pushinteger(L, part);
    lua::call_nothrow(L, 3, 0);
    lua::pop(L);
}

void scripting::on_entities_render(float deltaTime) {
    auto L = lua::get_main_thread();
    lua::get_from(L, STDCOMP, "render", true);
    lua::pushnumber(L, deltaTime);
    lua::call_nothrow(L, 1, 0);
    lua::pop(L);
}

void scripting::on_ui_open(
    UIDocument* layout,
    std::vector<dynamic::Value> args
) {
    auto argsptr = std::make_shared<std::vector<dynamic::Value>>(std::move(args));
    std::string name = layout->getId() + ".open";
    lua::emit_event(lua::get_main_thread(), name, [=] (auto L) {
        for (const auto& value : *argsptr) {
            lua::pushvalue(L, value);
        }
        return argsptr->size();
    });
}

void scripting::on_ui_progress(UIDocument* layout, int workDone, int workTotal) {
    std::string name = layout->getId() + ".progress";
    lua::emit_event(lua::get_main_thread(), name, [=] (auto L) {
        lua::pushinteger(L, workDone);
        lua::pushinteger(L, workTotal);
        return 2;
    });
}

void scripting::on_ui_close(UIDocument* layout, Inventory* inventory) {
    std::string name = layout->getId() + ".close";
    lua::emit_event(lua::get_main_thread(), name, [inventory] (auto L) {
        return lua::pushinteger(L, inventory ? inventory->getId() : 0);
    });
}

bool scripting::register_event(int env, const std::string& name, const std::string& id) {
    auto L = lua::get_main_thread();
    if (lua::pushenv(L, env) == 0) {
        lua::pushglobals(L);
    }
    if (lua::getfield(L, name)) {
        lua::pop(L);
        lua::getglobal(L, "events");
        lua::getfield(L, "on");
        lua::pushstring(L, id);
        lua::getfield(L, name, -4);
        lua::call_nothrow(L, 2);
        lua::pop(L);

        lua::pushnil(L);
        lua::setfield(L, name);
        return true;
    }
    return false;
}

int scripting::get_values_on_stack() {
    return lua::gettop(lua::get_main_thread());
}

static void load_script(int env, const std::string& type, const std::filesystem::path& file) {
    std::string src = files::read_string(file);
    LOG_INFO("Script ({}) {}", type, file.u8string());
    lua::execute(lua::get_main_thread(), env, src, file.u8string());
}

void scripting::load_block_script(const scriptenv& senv, const std::string& prefix, const std::filesystem::path& file, block_funcs_set& funcsset) {
    int env = *senv;
    load_script(env, "block", file);
    funcsset.init = register_event(env, "init", prefix + ".init");
    funcsset.update = register_event(env, "on_update", prefix + ".update");
    funcsset.randupdate = register_event(env, "on_random_update", prefix + ".randupdate");
    funcsset.onbroken = register_event(env, "on_broken", prefix + ".broken");
    funcsset.onplaced = register_event(env, "on_placed", prefix + ".placed");
    funcsset.oninteract = register_event(env, "on_interact", prefix + ".interact");
    funcsset.onblocksspark = register_event(env, "on_blocks_spark", prefix + ".blocksspark");
}

void scripting::load_item_script(const scriptenv& senv, const std::string& prefix, const std::filesystem::path& file, item_funcs_set& funcsset) {
    int env = *senv;
    load_script(env, "item", file);
    funcsset.init = register_event(env, "init", prefix + ".init");
    funcsset.on_use = register_event(env, "on_use", prefix + ".use");
    funcsset.on_use_on_block = register_event(env, "on_use_on_block", prefix + ".useon");
    funcsset.on_block_break_by = register_event(env, "on_block_break_by", prefix + ".blockbreakby");
}

void scripting::load_entity_component(const std::string& name, const std::filesystem::path& file) {
    auto L = lua::get_main_thread();
    std::string src = files::read_string(file);
    LOG_INFO("Script (entity component) {}", file.u8string());
    lua::loadbuffer(L, 0, src, "C!" + name);
    lua::store_in(L, lua::CHUNKS_TABLE, name);
}

void scripting::load_world_script(
    const scriptenv& senv,
    const std::string& prefix,
    const std::filesystem::path& file,
    world_funcs_set& funcsset
) {
    int env = *senv;
    load_script(env, "world", file);
    register_event(env, "init", prefix + ".init");
    register_event(env, "on_world_open", prefix + ".worldopen");
    register_event(env, "on_world_spark", prefix + ".worldspark");
    register_event(env, "on_world_save", prefix + ".worldsave");
    register_event(env, "on_world_quit", prefix + ".worldquit");

    funcsset.onblockplaced = register_event(env, "on_block_placed", prefix + ".blockplaced");
    funcsset.onblockbroken = register_event(env, "on_block_broken", prefix + ".blockbroken");
}

void scripting::load_layout_script(const scriptenv& senv, const std::string& prefix, const std::filesystem::path& file, uidocscript& script) {
    int env = *senv;
    load_script(env, "layout", file);
    script.onopen = register_event(env, "on_open", prefix + ".open");
    script.onprogress = register_event(env, "on_progress", prefix + ".progress");
    script.onclose = register_event(env, "on_close", prefix + ".close");
}

void scripting::close() {
    lua::finalize();
    content = nullptr;
    indices = nullptr;
    level = nullptr;
}
