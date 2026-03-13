#include "scripting.h"

#include <stdexcept>

#include "../../files/engine_paths.h"
#include "../../files/files.h"
#include "../../util/timeutil.h"
#include "../../world/Level.h"
#include "../../voxels/Block.h"
#include "../../logger/Logger.h"
#include "../../items/Item.h"
#include "../../logic/BlocksController.h"
#include "../../engine.h"
#include "../../content/ContentPack.h"
#include "lua/LuaState.h"
#include "../../util/stringutil.h"
#include "../../frontend/UIDocument.h"
#include "../../items/Inventory.h"
#include "../../objects/Player.h"

using namespace scripting;

namespace scripting {
    extern lua::LuaState* state;
}

Engine* scripting::engine = nullptr;
lua::LuaState* scripting::state = nullptr;
Level* scripting::level = nullptr;
const Content* scripting::content = nullptr;
BlocksController* scripting::blocks = nullptr;
const ContentIndices* scripting::indices = nullptr;

Environment::Environment(int env) : env(env) {
}

Environment::~Environment() {
    if (env) state->removeEnvironment(env);
}

int Environment::getId() const {
    return env;
}

bool scripting::register_event(int env, const std::string& name, const std::string& id) {
    if (state->pushenv(env) == 0) state->pushglobals();

    if (state->getfield(name)) {
        state->pushnil();
        state->setfield(name, -3);

        state->setglobal(id);
        state->pop();
        return true;
    }
    return false;
}

void load_script(std::filesystem::path name) {
    auto paths = scripting::engine->getPaths();
    std::filesystem::path file = paths->getResources()/std::filesystem::path("scripts")/name;
    std::string src = files::read_string(file);
    state->execute(0, src, file.u8string());
}

void scripting::initialize(Engine* engine) {
    scripting::engine = engine;
    state = new lua::LuaState();
    load_script(std::filesystem::path("stdlib.lua"));
}

static bool processCallback(int env, const std::string& src, const std::string& file) {
    try {
        return state->eval(env, src, file) != 0;
    } catch (const lua::luaerror& err) {
        LOG_ERROR("{}", err.what());
        return false;
    }
}

std::unique_ptr<Environment> scripting::create_environment(int parent) {
    return std::make_unique<Environment>(state->createEnvironment(parent));
}

std::unique_ptr<Environment> scripting::create_pack_environment(const ContentPack& pack) {
    int id = state->createEnvironment(0);
    state->pushenv(id);
    state->pushvalue(-1);
    state->setfield("PACK_ENV");
    state->pushstring(pack.id);
    state->setfield("PACK_ID");
    state->pop();
    return std::make_unique<Environment>(id);
}

std::unique_ptr<Environment> scripting::create_doc_environment(int parent, const std::string& name) {
    int id = state->createEnvironment(parent);
    state->pushenv(id);
    state->pushvalue(-1);
    state->setfield("DOC_ENV");
    state->pushstring(name.c_str());
    state->setfield("DOC_NAME");

    if (state->getglobal("Document")) {
        if (state->getfield("new")) {
            state->pushstring(name.c_str());
            if (state->callNoThrow(1)) state->setfield("document", -3);
        }
        state->pop();
    }
    state->pop();

    return std::make_unique<Environment>(id);
}

void scripting::on_world_load(Level* level, BlocksController* blocks) {
    scripting::level = level;
    scripting::content = level->content;
    scripting::indices = level->content->getIndices();
    scripting::blocks = blocks;
    load_script("world.lua");

    for (auto& pack : scripting::engine->getContentPacks()) {
        if (state->getglobal(pack.id + ".worldopen")) state->callNoThrow(0);
    }
}

void scripting::on_world_save() {
    for (auto& pack : scripting::engine->getContentPacks()) {
        if (state->getglobal(pack.id + ".worldsave")) state->callNoThrow(0);
    }
}

void scripting::on_world_quit() {
    for (auto& pack : scripting::engine->getContentPacks()) {
        if (state->getglobal(pack.id + ".worldquit")) state->callNoThrow(0);
    }
    if (state->getglobal("__scripts_cleanup")) state->callNoThrow(0);
    scripting::level = nullptr;
    scripting::content = nullptr;
    scripting::indices = nullptr;
}

void scripting::on_blocks_tick(const Block* block, int tps) {
    std::string name = block->name + ".blockstick";
    if (state->getglobal(name)) {
        state->pushinteger(tps);
        state->callNoThrow(1);
    }
}

void scripting::update_block(const Block* block, int x, int y, int z) {
    std::string name = block->name + ".update";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->callNoThrow(3);
    }
}

void scripting::random_update_block(const Block* block, int x, int y, int z) {
    std::string name = block->name + ".randupdate";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->callNoThrow(3);
    }
}

void scripting::on_block_placed(Player* player, const Block* block, int x, int y, int z) {
    std::string name = block->name + ".placed";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->pushinteger(player->getId());
        state->callNoThrow(4);
    }
}

void scripting::on_block_broken(Player* player, const Block* block, int x, int y, int z) {
    std::string name = block->name + ".broken";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->pushinteger(player->getId());
        state->callNoThrow(4);
    }
}

bool scripting::on_block_interact(Player* player, const Block* block, int x, int y, int z) {
    std::string name = block->name + ".interact";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->pushinteger(player->getId());
        if (state->callNoThrow(4)) return state->toboolean(-1);
    }
    return false;
}

bool scripting::on_item_use_on_block(Player* player, const Item* item, int x, int y, int z) {
    std::string name = item->name + ".useon";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->pushinteger(player->getId());
        if (state->callNoThrow(4)) return state->toboolean(-1);
    }
    return false;
}

bool scripting::on_item_break_block(Player* player, const Item* item, int x, int y, int z) {
    std::string name = item->name + ".blockbreakby";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->pushinteger(player->getId());
        if (state->callNoThrow(4)) return state->toboolean(-1);
    }
    return false;
}

void scripting::on_ui_open(UIDocument* layout, Inventory* inventory, glm::ivec3 blockcoord) {
    std::string name = layout->getId() + ".open";
    if (state->getglobal(name)) {
        state->pushinteger(inventory == nullptr ? 0 : inventory->getId());
        state->pushivec3(blockcoord.x, blockcoord.y, blockcoord.z);
        state->callNoThrow(4);
    }
}

void scripting::on_ui_close(UIDocument* layout, Inventory* inventory) {
    std::string name = layout->getId()+".close";
    if (state->getglobal(name)) {
        state->pushinteger(inventory->getId());
        state->callNoThrow(1);
    }
}

void scripting::load_block_script(int env, std::string prefix, std::filesystem::path file, block_funcs_set& funcsset) {
    std::string src = files::read_string(file);

    LOG_DEBUG("Loading script {}", file.u8string());

    state->execute(env, src, file.u8string());

    state->execute(env, src, file.u8string());
    funcsset.init = register_event(env, "init", prefix + ".init");
    funcsset.update = register_event(env, "on_update", prefix + ".update");
    funcsset.randupdate = register_event(env, "on_random_update", prefix + ".randupdate");
    funcsset.onbroken = register_event(env, "on_broken", prefix + ".broken");
    funcsset.onplaced = register_event(env, "on_placed", prefix + ".placed");
    funcsset.oninteract = register_event(env, "on_interact", prefix + ".interact");
    funcsset.onblockstick = register_event(env, "on_blocks_tick", prefix + ".blockstick");

    LOG_DEBUG("Script {} successfully loaded", file.u8string());
}

void scripting::load_item_script(int env, std::string prefix, std::filesystem::path file, item_funcs_set& funcsset) {
    std::string src = files::read_string(file);
    LOG_DEBUG("Loading script {}", file.u8string());
    state->execute(env, src, file.u8string());
    funcsset.init = register_event(env, "init", prefix + ".init");
    funcsset.on_use_on_block = register_event(env, "on_use_on_block", prefix + ".useon");
    funcsset.on_block_break_by = register_event(env, "on_block_break_by", prefix + ".blockbreakby");
    LOG_DEBUG("Script {} successfully loaded", file.u8string());
}

void scripting::load_world_script(int env, std::string prefix, std::filesystem::path file) {
    std::string src = files::read_string(file);
    LOG_DEBUG("Loading script {}", file.u8string());

    state->loadbuffer(env, src, file.u8string());
    state->callNoThrow(0);

    register_event(env, "init", prefix + ".init");
    register_event(env, "on_world_open", prefix + ".worldopen");
    register_event(env, "on_world_save", prefix + ".worldsave");
    register_event(env, "on_world_quit", prefix + ".worldquit");

    LOG_DEBUG("Script {} successfully loaded", file.u8string());
}

void scripting::load_layout_script(int env, std::string prefix, std::filesystem::path file, uidocscript& script) {
    std::string src = files::read_string(file);
    LOG_DEBUG("Loading script {}", file.u8string());
    script.environment = env;
    state->loadbuffer(env, src, file.u8string());
    state->callNoThrow(0);
    script.onopen = register_event(env, "on_open", prefix + ".open");
    script.onclose = register_event(env, "on_close", prefix + ".close");
    LOG_DEBUG("Script {} successfully loaded", file.u8string());
}

void scripting::close() {
    delete state;

    state = nullptr;
    content = nullptr;
    level = nullptr;
}
