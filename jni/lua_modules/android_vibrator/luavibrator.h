#ifndef luavibrator_h
#define luavibrator_h

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

#define LUA_VIBRATORLIBNAME "vibrator"

LUALIB_API int (luaopen_vibrator) (lua_State *L);

#endif
