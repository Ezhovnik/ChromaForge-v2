#include "api_lua.h"

#include "scripting.h"
#include "../../world/Level.h"
#include "../../content/Content.h"
#include "../../voxels/Block.h"
#include "../../voxels/Chunks.h"
#include "../../voxels/voxel.h"

int l_block_name(lua_State* L) {
    int id = lua_tointeger(L, 1);
    lua_pushstring(L, scripting::content->indices->getBlockDef(id)->name.c_str());
    return 1;
}

int l_is_solid_at(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);

    lua_pushboolean(L, scripting::level->chunks->isSolid(x, y, z));
    return 1;
}

int l_blocks_count(lua_State* L) {
    lua_pushinteger(L, scripting::content->indices->countBlockDefs());
    return 1;
}

int l_block_index(lua_State* L) {
    auto name = lua_tostring(L, 1);
    lua_pushinteger(L, scripting::content->requireBlock(name)->rt.id);
    return 1;
}

int l_set_block(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    int id = lua_tointeger(L, 4);    
    scripting::level->chunks->setVoxel(x, y, z, id, 0);
    return 0;
}

int l_get_block(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->getVoxel(x, y, z);
    int id = vox == nullptr ? -1 : vox->id;
    lua_pushinteger(L, id);
    return 1;
}

void lua_addfunc(lua_State* L, lua_CFunction func, const char* name) {
    lua_pushcfunction(L, func);
    lua_setglobal(L, name);
}

void apilua::create_funcs(lua_State* L) {
    lua_addfunc(L, l_block_index, "block_index");
    lua_addfunc(L, l_block_name, "block_name");
    lua_addfunc(L, l_blocks_count, "blocks_count");
    lua_addfunc(L, l_is_solid_at, "is_solid_at");
    lua_addfunc(L, l_set_block, "set_block");
    lua_addfunc(L, l_get_block, "get_block");
}
