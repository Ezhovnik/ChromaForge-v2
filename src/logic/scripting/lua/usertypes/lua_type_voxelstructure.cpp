#include <logic/scripting/lua/lua_custom_types.h>

#include <logic/scripting/lua/lua_util.h>
#include <world/generator/VoxelFragment.h>
#include <util/stringutil.h>

using namespace lua;

LuaVoxelStructure::LuaVoxelStructure(std::shared_ptr<VoxelFragment> structure) : structure(std::move(structure)) {}

LuaVoxelStructure::~LuaVoxelStructure() {
}

static int l_meta_tostring(lua::State* L) {
    return pushstring(L, "VoxelFragment(0x" + util::tohex(
        reinterpret_cast<uint64_t>(topointer(L, 1))) + ")"
    );
}

int LuaVoxelStructure::createMetatable(lua::State* L) {
    createtable(L, 0, 1);
    pushcfunction(L, lua::wrap<l_meta_tostring>);
    setfield(L, "__tostring");
    return 1;
}
