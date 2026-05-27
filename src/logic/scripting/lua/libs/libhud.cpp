#include <logic/scripting/lua/libs/api_lua.h>
#include <assets/Assets.h>
#include <frontend/hud.h>
#include <world/Level.h>
#include <voxels/Chunks.h>
#include <voxels/voxel.h>
#include <voxels/Block.h>
#include <content/Content.h>
#include <logic/BlocksController.h>
#include <items/Inventories.h>
#include <engine.h>
#include <frontend/UIDocument.h>
#include <graphics/ui/elements/display/InventoryView.h>
#include <util/stringutil.h>
#include <objects/Player.h>

namespace scripting {
    extern Hud* hud;
}

static int l_open_inventory(lua::State*) {
    if (!scripting::hud->isInventoryOpen()) scripting::hud->openInventory();
    return 0;
}

static int l_close_inventory(lua::State*) {
    if (scripting::hud->isInventoryOpen()) scripting::hud->closeInventory();
    return 0;
}

static int l_open(lua::State* L) {
    auto invid = lua::tointeger(L, 1);
    auto layoutid = lua::require_string(L, 2);
    bool playerInventory = !lua::toboolean(L, 3);

    auto assets = scripting::engine->getAssets();
    auto layout = assets->get<UIDocument>(layoutid);
    if (layout == nullptr) {
        throw std::runtime_error("There is no ui layout " + util::quote(layoutid));
    }

    scripting::hud->openInventory(
        layout,
        scripting::level->inventories->get(invid),
        playerInventory
    );

    return 0;
}

static int l_open_block(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    bool playerInventory = !lua::toboolean(L, 4);

    voxel* vox = scripting::level->chunks->getVoxel(x, y, z);
    if (vox == nullptr) {
        throw std::runtime_error("Block does not exists at " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z));
    }
    auto& def = scripting::content->getIndices()->blocks.require(vox->id);
    auto assets = scripting::engine->getAssets();
    auto layout = assets->get<UIDocument>(def.uiLayout);
    if (layout == nullptr) {
        throw std::runtime_error("Block '" + def.name+  "' has no ui layout");
    }

    auto id = scripting::blocks->createBlockInventory(x, y, z);

    scripting::hud->openInventory(glm::ivec3(x, y, z), layout, scripting::level->inventories->get(id), playerInventory);

    lua::pushinteger(L, id);
    lua::pushstring(L, def.uiLayout);
    return 2;
}

static int l_show_overlay(lua::State* L) {
    auto name = lua::require_string(L, 1);
    bool playerInventory = lua::toboolean(L, 2);

    auto assets = scripting::engine->getAssets();
    auto layout = assets->get<UIDocument>(name);
    if (layout == nullptr) {
        throw std::runtime_error("There is no ui layout " + util::quote(name));
    }
    auto args = lua::tovalue(L, 3);
    scripting::hud->showOverlay(layout, playerInventory, std::move(args));
    return 0;
}

static UIDocument* require_layout(const char* name) {
    auto assets = scripting::engine->getAssets();
    auto layout = assets->get<UIDocument>(name);
    if (layout == nullptr) {
        throw std::runtime_error("Layout '" + std::string(name) + "' is not found");
    }
    return layout;
}

static int l_open_permanent(lua::State* L) {
    auto layout = require_layout(lua::require_string(L, 1));
    scripting::hud->openPermanent(layout);
    return 0;
}

static int l_close(lua::State* L) {
    auto layout = require_layout(lua::require_string(L, 1));
    scripting::hud->remove(layout->getRoot());
    return 0;
}

static int l_pause(lua::State*) {
    scripting::hud->setPause(true);
    return 0;
}

static int l_resume(lua::State*) {
    scripting::hud->setPause(false);
    return 0;
}

static int l_get_block_inventory(lua::State* L) {
    auto inventory = scripting::hud->getBlockInventory();
    if (inventory == nullptr) {
        return lua::pushinteger(L, 0);
    } else {
        return lua::pushinteger(L, inventory->getId());
    }
}

static int l_get_player(lua::State* L) {
    auto player = scripting::hud->getPlayer();
    return lua::pushinteger(L, player->getId());
}

static int l_is_paused(lua::State* L) {
    return lua::pushboolean(L, scripting::hud->isPause());
}

static int l_is_inventory_open(lua::State* L) {
    return lua::pushboolean(L, scripting::hud->isInventoryOpen());
}

static int l_is_content_access(lua::State* L) {
    return lua::pushboolean(L, scripting::hud->isContentAccess());
}

static int l_set_content_access(lua::State* L) {
    scripting::hud->setContentAccess(lua::toboolean(L, 1));
    return 0;
}

static int l_set_debug_cheats(lua::State* L) {
    scripting::hud->setDebugCheats(lua::toboolean(L, 1));
    return 0;
}

const luaL_Reg hudlib [] = {
    {"open_inventory", lua::wrap<l_open_inventory>},
    {"close_inventory", lua::wrap<l_close_inventory>},
    {"open", lua::wrap<l_open>},
    {"open_block", lua::wrap<l_open_block>},
    {"open_permanent", lua::wrap<l_open_permanent>},
    {"show_overlay", lua::wrap<l_show_overlay>},
    {"get_block_inventory", lua::wrap<l_get_block_inventory>},
    {"close", lua::wrap<l_close>},
    {"pause", lua::wrap<l_pause>},
    {"resume", lua::wrap<l_resume>},
    {"is_paused", lua::wrap<l_is_paused>},
    {"is_inventory_open", lua::wrap<l_is_inventory_open>},
    {"get_player", lua::wrap<l_get_player>},
    {"_is_content_access", lua::wrap<l_is_content_access>},
    {"_set_content_access", lua::wrap<l_set_content_access>},
    {"_set_debug_cheats", lua::wrap<l_set_debug_cheats>},
    {NULL, NULL}
};
