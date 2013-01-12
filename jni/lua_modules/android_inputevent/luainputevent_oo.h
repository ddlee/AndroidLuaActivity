#ifndef luainputevent_h
#define luainputevent_h

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

LUALIB_API int luaopen_inputevent (lua_State *L);

LUALIB_API
void lua_pushinputevent(lua_State *L, const AInputEvent *event);

#endif
