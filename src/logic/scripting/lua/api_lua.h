#ifndef LOGIC_SCRIPTING_LUA_API_LUA_H_
#define LOGIC_SCRIPTING_LUA_API_LUA_H_

#include "lua_util.h"

extern const luaL_Reg packlib [];
extern const luaL_Reg timelib [];
extern const luaL_Reg filelib [];
extern const luaL_Reg worldlib [];
extern const luaL_Reg blocklib [];
extern const luaL_Reg itemlib [];
extern const luaL_Reg playerlib [];
extern const luaL_Reg inventorylib [];
extern const luaL_Reg guilib [];
extern const luaL_Reg hudlib [];
extern const luaL_Reg audiolib [];
extern const luaL_Reg builtinlib [];
extern const luaL_Reg jsonlib [];
extern const luaL_Reg inputlib [];
extern const luaL_Reg consolelib [];
extern const luaL_Reg tomllib [];
extern const luaL_Reg mat4lib [];
extern const luaL_Reg entitylib [];
extern const luaL_Reg vec2lib [];
extern const luaL_Reg vec3lib [];
extern const luaL_Reg vec4lib [];

extern const luaL_Reg skeletonlib [];
extern const luaL_Reg rigidbodylib [];
extern const luaL_Reg transformlib [];

extern int l_print(lua::State* L);

#endif // LOGIC_SCRIPTING_LUA_API_LUA_H_
