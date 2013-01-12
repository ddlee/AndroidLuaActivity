#ifndef luatts_h
#define luatts_h

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

LUALIB_API int (luaopen_tts) (lua_State *L);

#endif
