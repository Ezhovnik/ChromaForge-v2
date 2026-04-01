#include "lua_commons.h"
#include "api_lua.h"
#include "../scripting.h"
#include "../../../assets/Assets.h"
#include "../../../frontend/hud.h"
#include "../../../world/Level.h"
#include "../../../voxels/Chunks.h"
#include "../../../voxels/voxel.h"
#include "../../../voxels/Block.h"
#include "../../../content/Content.h"
#include "../../../logic/BlocksController.h"
#include "../../../items/Inventories.h"
#include "../../../engine.h"
#include "LuaState.h"
#include "../../../frontend/UIDocument.h"
#include "../../../graphics/ui/elements/display/InventoryView.h"
#include "../../../util/stringutil.h"
#include "../../../debug/Logger.h"
#include "../../../objects/Player.h"

namespace scripting {
    extern Hud* hud;
}

static int l_hud_open_inventory(lua_State*) {
    if (!scripting::hud->isInventoryOpen()) scripting::hud->openInventory();
    return 0;
}

static int l_hud_close_inventory(lua_State*) {
    if (scripting::hud->isInventoryOpen()) scripting::hud->closeInventory();
    return 0;
}

static int l_hud_open_block(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);
    bool playerInventory = !lua_toboolean(L, 4);

    voxel* vox = scripting::level->chunks->getVoxel(x, y, z);
    if (vox == nullptr) {
        throw std::runtime_error("Block does not exists at " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z));
    }
    auto def = scripting::content->getIndices()->getBlockDef(vox->id);
    auto assets = scripting::engine->getAssets();
    auto layout = assets->getLayout(def->uiLayout);
    if (layout == nullptr) {
        throw std::runtime_error("Block '" + def->name+  "' has no ui layout");
    }

    auto id = scripting::blocks->createBlockInventory(x, y, z);

    scripting::hud->openInventory(glm::ivec3(x, y, z), layout, scripting::level->inventories->get(id), playerInventory);

    lua_pushinteger(L, id);
    lua_pushstring(L, def->uiLayout.c_str());
    return 2;
}

static int l_hud_show_overlay(lua_State* L) {
    const char* name = lua_tostring(L, 1);
    bool playerInventory = lua_toboolean(L, 2);

    auto assets = scripting::engine->getAssets();
    auto layout = assets->getLayout(name);
    if (layout == nullptr) {
        throw std::runtime_error("There is no ui layout " + util::quote(name));
    }
    scripting::hud->showOverlay(layout, playerInventory);
    return 0;
}

static UIDocument* require_layout(lua_State* L, const char* name) {
    auto assets = scripting::engine->getAssets();
    auto layout = assets->getLayout(name);
    if (layout == nullptr) {
        throw std::runtime_error("Layout '" + std::string(name) + "' is not found");
    }
    return layout;
}

static int l_hud_open_permanent(lua_State* L) {
    auto layout = require_layout(L, lua_tostring(L, 1));
    scripting::hud->openPermanent(layout);
    return 0;
}

static int l_hud_close(lua_State* L) {
    auto layout = require_layout(L, lua_tostring(L, 1));
    scripting::hud->remove(layout->getRoot());
    return 0;
}

static int l_hud_pause(lua_State*) {
    scripting::hud->setPause(true);
    return 0;
}

static int l_hud_resume(lua_State*) {
    scripting::hud->setPause(false);
    return 0;
}

static int l_hud_get_block_inventory(lua_State* L) {
    auto inventory = scripting::hud->getBlockInventory();
    if (inventory == nullptr) {
        lua_pushinteger(L, 0);
    } else {
        lua_pushinteger(L, inventory->getId());
    }
    return 1;
}

static int l_hud_get_player(lua_State* L) {
    auto player = scripting::hud->getPlayer();
    lua_pushinteger(L, player->getId());
    return 1;
}

const luaL_Reg hudlib [] = {
    {"open_inventory", lua_wrap_errors<l_hud_open_inventory>},
    {"close_inventory", lua_wrap_errors<l_hud_close_inventory>},
    {"open_block", lua_wrap_errors<l_hud_open_block>},
    {"show_overlay", lua_wrap_errors<l_hud_show_overlay>},
    {"open_permanent", lua_wrap_errors<l_hud_open_permanent>},
    {"close", lua_wrap_errors<l_hud_close>},
    {"pause", lua_wrap_errors<l_hud_pause>},
    {"resume", lua_wrap_errors<l_hud_resume>},
    {"get_block_inventory", lua_wrap_errors<l_hud_get_block_inventory>},
    {"get_player", lua_wrap_errors<l_hud_get_player>},
    {NULL, NULL}
};
