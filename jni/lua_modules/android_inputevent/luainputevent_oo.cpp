/*
  Lua module to manipulate Android InputEvent
*/

#include <android/input.h>
#include <android/log.h>

#ifdef __cplusplus
extern "C"
{
#endif
  #include "lua.h"
  #include "lauxlib.h"
#ifdef __cplusplus
}
#endif

#include "luainputevent.h"

#define MT_NAME "inputevent_mt"

static AInputEvent* lua_checkinputevent(lua_State *L, int narg) {
  void *ud = luaL_checkudata(L, narg, MT_NAME);
  luaL_argcheck(L, *(AInputEvent **)ud != NULL, narg, "invalid object");
  return *(AInputEvent **)ud;
}

extern "C"
void lua_pushinputevent(lua_State *L, const AInputEvent *event) {
  AInputEvent **ud =
    (AInputEvent **)lua_newuserdata(L, sizeof(AInputEvent *));
  *ud = (AInputEvent *)event;
  luaL_getmetatable(L, MT_NAME);
  lua_setmetatable(L, -2);
}

static int lua_inputevent_new(lua_State *L) {
  if (!lua_islightuserdata(L, 1)) {
    return luaL_error(L, "not lightuserdata");
  }
  AInputEvent *event = (AInputEvent *)lua_touserdata(L, 1);
  AInputEvent **ud =
    (AInputEvent **)lua_newuserdata(L, sizeof(AInputEvent *));
  *ud = event;
  luaL_getmetatable(L, MT_NAME);
  lua_setmetatable(L, -2);
}

static int lua_inputevent_delete(lua_State *L) {
  AInputEvent *event = lua_checkinputevent(L, 1);
  return 0;
}

static int lua_inputevent_get(lua_State *L) {
  AInputEvent *event = lua_checkinputevent(L, 1);
  const char *key = luaL_checkstring(L, 2);
  
  lua_pushinteger(L, 0);
  return 1;
}

//Generic Input Event accessors:

static int lua_inputevent_getType(lua_State *L) {
  AInputEvent *event = lua_checkinputevent(L, 1);
  int type = AInputEvent_getType(event);
  lua_pushinteger(L, type);
  return 1;
}

static int lua_inputevent_getDeviceId(lua_State *L) {
  AInputEvent *event = lua_checkinputevent(L, 1);
  int deviceid = AInputEvent_getType(event);
  lua_pushinteger(L, deviceid);
  return 1;
}

static int lua_inputevent_getSource(lua_State *L) {
  AInputEvent *event = lua_checkinputevent(L, 1);
  int source = AInputEvent_getType(event);
  lua_pushinteger(L, source);
  return 1;
}

static int lua_inputevent_tostring(lua_State *L) {
  AInputEvent *event = lua_checkinputevent(L, 1);
  lua_pushfstring(L, "InputEvent(%p): type %d, device %d, source %d",
		  event,
		  AInputEvent_getType(event),
		  AInputEvent_getDeviceId(event),
		  AInputEvent_getSource(event));
  return 1;
}

// Key events
static int lua_inputevent_getKeyCode(lua_State *L) {
  AInputEvent *event = lua_checkinputevent(L, 1);
  int keycode = AKeyEvent_getKeyCode(event);
  lua_pushinteger(L, keycode);
  return 1;
}

static const struct luaL_reg inputevent_functions[] = {
  {"new", lua_inputevent_new},
  {NULL, NULL}
};

static const struct luaL_reg inputevent_methods[] = {
  {"getType", lua_inputevent_getType},
  {"getDeviceId", lua_inputevent_getDeviceId},
  {"getSource", lua_inputevent_getSource},
  {"getKeyCode", lua_inputevent_getKeyCode},
  {"__gc", lua_inputevent_delete},
  {"__tostring", lua_inputevent_tostring},
  {NULL, NULL}
};

extern "C"
int luaopen_inputevent (lua_State *L) {

  luaL_newmetatable(L, MT_NAME);
  // OO access: mt.__index = mt
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  /*
  // Array access: mt.__index = get();
  // Not compatible with OO access
  lua_pushstring(L, "__index");
  lua_pushcfunction(L, lua_inputevent_get);
  lua_settable(L, -3);
  */

  luaL_register(L, NULL, inputevent_methods);
  luaL_register(L, "inputevent", inputevent_functions);
  return 1;
}
