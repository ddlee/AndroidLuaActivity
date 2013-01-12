#ifndef luatoast_h
#define luatoast_h

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

LUALIB_API int (luaopen_toast) (lua_State *L);

#endif
