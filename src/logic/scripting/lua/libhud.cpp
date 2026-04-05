#include "api_lua.h"
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
#include "../../../frontend/UIDocument.h"
#include "../../../graphics/ui/elements/display/InventoryView.h"
#include "../../../util/stringutil.h"
#include "../../../objects/Player.h"

namespace scripting {
    extern Hud* hud;
}

static int l_hud_open_inventory(lua::State*) {
    if (!scripting::hud->isInventoryOpen()) scripting::hud->openInventory();
    return 0;
}

static int l_hud_close_inventory(lua::State*) {
    if (scripting::hud->isInventoryOpen()) scripting::hud->closeInventory();
    return 0;
}

static int l_hud_open_block(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    bool playerInventory = !lua::toboolean(L, 4);

    voxel* vox = scripting::level->chunks->getVoxel(x, y, z);
    if (vox == nullptr) {
        throw std::runtime_error("Block does not exists at " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z));
    }
    auto def = scripting::content->getIndices()->blocks.get(vox->id);
    auto assets = scripting::engine->getAssets();
    auto layout = assets->get<UIDocument>(def->uiLayout);
    if (layout == nullptr) {
        throw std::runtime_error("Block '" + def->name+  "' has no ui layout");
    }

    auto id = scripting::blocks->createBlockInventory(x, y, z);

    scripting::hud->openInventory(glm::ivec3(x, y, z), layout, scripting::level->inventories->get(id), playerInventory);

    lua::pushinteger(L, id);
    lua::pushstring(L, def->uiLayout);
    return 2;
}

static int l_hud_show_overlay(lua::State* L) {
    auto name = lua::tostring(L, 1);
    bool playerInventory = lua::toboolean(L, 2);

    auto assets = scripting::engine->getAssets();
    auto layout = assets->get<UIDocument>(name);
    if (layout == nullptr) {
        throw std::runtime_error("There is no ui layout " + util::quote(name));
    }
    scripting::hud->showOverlay(layout, playerInventory);
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

static int l_hud_open_permanent(lua::State* L) {
    auto layout = require_layout(lua::tostring(L, 1));
    scripting::hud->openPermanent(layout);
    return 0;
}

static int l_hud_close(lua::State* L) {
    auto layout = require_layout(lua::tostring(L, 1));
    scripting::hud->remove(layout->getRoot());
    return 0;
}

static int l_hud_pause(lua::State*) {
    scripting::hud->setPause(true);
    return 0;
}

static int l_hud_resume(lua::State*) {
    scripting::hud->setPause(false);
    return 0;
}

static int l_hud_get_block_inventory(lua::State* L) {
    auto inventory = scripting::hud->getBlockInventory();
    if (inventory == nullptr) {
        return lua::pushinteger(L, 0);
    } else {
        return lua::pushinteger(L, inventory->getId());
    }
}

static int l_hud_get_player(lua::State* L) {
    auto player = scripting::hud->getPlayer();
    return lua::pushinteger(L, player->getId());
}

const luaL_Reg hudlib [] = {
    {"open_inventory", lua::wrap<l_hud_open_inventory>},
    {"close_inventory", lua::wrap<l_hud_close_inventory>},
    {"open_block", lua::wrap<l_hud_open_block>},
    {"open_permanent", lua::wrap<l_hud_open_permanent>},
    {"show_overlay", lua::wrap<l_hud_show_overlay>},
    {"get_block_inventory", lua::wrap<l_hud_get_block_inventory>},
    {"close", lua::wrap<l_hud_close>},
    {"pause", lua::wrap<l_hud_pause>},
    {"resume", lua::wrap<l_hud_resume>},
    {"get_player", lua::wrap<l_hud_get_player>},
    {NULL, NULL}
};
