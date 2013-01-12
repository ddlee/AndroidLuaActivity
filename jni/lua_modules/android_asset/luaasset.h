#ifndef luaasset_h
#define luaasset_h

#ifdef __cplusplus
extern "C"
{
#endif
  #include "lua.h"
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#undef LUALIB_API
#define LUALIB_API extern "C"
#endif

#define LUA_ASSETLIBNAME "asset"

LUALIB_API int (luaopen_asset) (lua_State *L);
LUALIB_API int (luaL_loadasset) (lua_State *L, const char *filename);

#define luaL_doasset(L, fn) \
  (luaL_loadasset(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#endif
