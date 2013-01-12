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

typedef struct luaIntConst {
  const char *key;
  int value;
} luaIntConst;

static luaIntConst inputeventConst[] = {
  { "AKEY_EVENT_ACTION_DOWN", AKEY_EVENT_ACTION_DOWN},
  { "AKEY_EVENT_ACTION_UP", AKEY_EVENT_ACTION_UP},
  { "AKEY_EVENT_ACTION_MULTIPLE", AKEY_EVENT_ACTION_MULTIPLE},
  { "AKEY_STATE_UNKNOWN", AKEY_STATE_UNKNOWN},
  { "AKEY_STATE_UP", AKEY_STATE_UP},
  { "AKEY_STATE_DOWN", AKEY_STATE_DOWN},
  { "AKEY_STATE_VIRTUAL", AKEY_STATE_VIRTUAL},
  { "AINPUT_EVENT_TYPE_KEY", AINPUT_EVENT_TYPE_KEY},
  { "AINPUT_EVENT_TYPE_MOTION", AINPUT_EVENT_TYPE_MOTION},
  { "AMOTION_EVENT_ACTION_DOWN", AMOTION_EVENT_ACTION_DOWN},
  { "AMOTION_EVENT_ACTION_UP", AMOTION_EVENT_ACTION_UP},
  { NULL, 0}
};

static const AInputEvent* lua_checkinputevent(lua_State *L, int narg) {
  if (!lua_islightuserdata(L, narg)) {
    luaL_error(L, "invalid input event pointer");
  }
  return (const AInputEvent *)lua_touserdata(L, narg);
}

//Generic Input Event accessors:

static int lua_inputevent_getType(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int type = AInputEvent_getType(event);
  lua_pushinteger(L, type);
  return 1;
}

static int lua_inputevent_getDeviceId(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int deviceid = AInputEvent_getType(event);
  lua_pushinteger(L, deviceid);
  return 1;
}

static int lua_inputevent_getSource(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int source = AInputEvent_getType(event);
  lua_pushinteger(L, source);
  return 1;
}

static int lua_inputevent_getAction(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int action;
  int type = AInputEvent_getType(event);
  switch(type) {
  case AINPUT_EVENT_TYPE_KEY:
    action = AKeyEvent_getAction(event);
    break;
  case AINPUT_EVENT_TYPE_MOTION:
    action = AMotionEvent_getAction(event);
    break;
  default:
    return luaL_error(L, "unknown event type %d", type);
  }
  lua_pushinteger(L, action);
  return 1;
}

static int lua_inputevent_getFlags(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int flags;
  int type = AInputEvent_getType(event);
  switch(type) {
  case AINPUT_EVENT_TYPE_KEY:
    flags = AKeyEvent_getFlags(event);
    break;
  case AINPUT_EVENT_TYPE_MOTION:
    flags = AMotionEvent_getFlags(event);
    break;
  default:
    return luaL_error(L, "unknown event type %d", type);
  }
  lua_pushinteger(L, flags);
  return 1;
}

static int lua_inputevent_getMetaState(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int meta;
  int type = AInputEvent_getType(event);
  switch(type) {
  case AINPUT_EVENT_TYPE_KEY:
    meta = AKeyEvent_getMetaState(event);
    break;
  case AINPUT_EVENT_TYPE_MOTION:
    meta = AMotionEvent_getMetaState(event);
    break;
  default:
    return luaL_error(L, "unknown event type %d", type);
  }
  lua_pushinteger(L, meta);
  return 1;
}

static int lua_inputevent_getDownTime(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int64_t t;
  int type = AInputEvent_getType(event);
  switch(type) {
  case AINPUT_EVENT_TYPE_KEY:
    t = AKeyEvent_getDownTime(event);
    break;
  case AINPUT_EVENT_TYPE_MOTION:
    t = AMotionEvent_getDownTime(event);
    break;
  default:
    return luaL_error(L, "unknown event type %d", type);
  }
  lua_pushnumber(L, (double)t);
  return 1;
}

static int lua_inputevent_getEventTime(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int64_t t;
  int type = AInputEvent_getType(event);
  switch(type) {
  case AINPUT_EVENT_TYPE_KEY:
    t = AKeyEvent_getEventTime(event);
    break;
  case AINPUT_EVENT_TYPE_MOTION:
    t = AMotionEvent_getEventTime(event);
    break;
  default:
    return luaL_error(L, "unknown event type %d", type);
  }
  lua_pushnumber(L, (double)t);
  return 1;
}

// Key events
static int lua_inputevent_getKeyCode(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  lua_pushinteger(L, AKeyEvent_getKeyCode(event));
  return 1;
}

static int lua_inputevent_getRepeatCount(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  lua_pushinteger(L, AKeyEvent_getRepeatCount(event));
  return 1;
}

// Motion events
static int lua_inputevent_getX(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int pointerCount = AMotionEvent_getPointerCount(event);
  lua_createtable(L, pointerCount, 0);
  for (int i = 0; i < pointerCount; i++) {
    //    int pointerId = AMotionEvent_getPointerId(event, i);
    lua_pushnumber(L, AMotionEvent_getX(event, i));
    lua_rawseti(L, -2, i+1);
  }
  return 1;
}

static int lua_inputevent_getY(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int pointerCount = AMotionEvent_getPointerCount(event);
  lua_createtable(L, pointerCount, 0);
  for (int i = 0; i < pointerCount; i++) {
    lua_pushnumber(L, AMotionEvent_getY(event, i));
    lua_rawseti(L, -2, i+1);
  }
  return 1;
}

static int lua_inputevent_getPressure(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int pointerCount = AMotionEvent_getPointerCount(event);
  lua_createtable(L, pointerCount, 0);
  for (int i = 0; i < pointerCount; i++) {
    lua_pushnumber(L, AMotionEvent_getPressure(event, i));
    lua_rawseti(L, -2, i+1);
  }
  return 1;
}

static int lua_inputevent_getSize(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int pointerCount = AMotionEvent_getPointerCount(event);
  lua_createtable(L, pointerCount, 0);
  for (int i = 0; i < pointerCount; i++) {
    lua_pushnumber(L, AMotionEvent_getSize(event, i));
    lua_rawseti(L, -2, i+1);
  }
  return 1;
}

static int lua_inputevent_getTouchMajor(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int pointerCount = AMotionEvent_getPointerCount(event);
  lua_createtable(L, pointerCount, 0);
  for (int i = 0; i < pointerCount; i++) {
    lua_pushnumber(L, AMotionEvent_getTouchMajor(event, i));
    lua_rawseti(L, -2, i+1);
  }
  return 1;
}

static int lua_inputevent_getTouchMinor(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int pointerCount = AMotionEvent_getPointerCount(event);
  lua_createtable(L, pointerCount, 0);
  for (int i = 0; i < pointerCount; i++) {
    lua_pushnumber(L, AMotionEvent_getTouchMinor(event, i));
    lua_rawseti(L, -2, i+1);
  }
  return 1;
}

static int lua_inputevent_getOrientation(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  int pointerCount = AMotionEvent_getPointerCount(event);
  lua_createtable(L, pointerCount, 0);
  for (int i = 0; i < pointerCount; i++) {
    lua_pushnumber(L, AMotionEvent_getOrientation(event, i));
    lua_rawseti(L, -2, i+1);
  }
  return 1;
}

static int lua_inputevent_tostring(lua_State *L) {
  const AInputEvent *event = lua_checkinputevent(L, 1);
  lua_pushfstring(L, "InputEvent(%p): type %d, device %d, source %d",
		  event,
		  AInputEvent_getType(event),
		  AInputEvent_getDeviceId(event),
		  AInputEvent_getSource(event));
  return 1;
}

static const struct luaL_reg inputevent_functions[] = {
  {"getType", lua_inputevent_getType},
  {"getDeviceId", lua_inputevent_getDeviceId},
  {"getSource", lua_inputevent_getSource},
  {"getAction", lua_inputevent_getAction},
  {"getFlags", lua_inputevent_getFlags},
  {"getMetaState", lua_inputevent_getMetaState},
  {"getDownTime", lua_inputevent_getDownTime},
  {"getEventTime", lua_inputevent_getEventTime},
  {"getKeyCode", lua_inputevent_getKeyCode},
  {"getX", lua_inputevent_getX},
  {"getY", lua_inputevent_getY},
  {"getPressure", lua_inputevent_getPressure},
  {"getSize", lua_inputevent_getSize},
  {"getTouchMajor", lua_inputevent_getTouchMajor},
  {"getTouchMinor", lua_inputevent_getTouchMinor},
  {"getOrientation", lua_inputevent_getOrientation},
  {NULL, NULL}
};

extern "C"
int luaopen_inputevent (lua_State *L) {
  luaL_register(L, "inputevent", inputevent_functions);

  // Initialize constants
  luaIntConst *c = inputeventConst;
  for (; c->key; c++) {
    lua_pushstring(L, c->key);
    lua_pushinteger(L, c->value);
    lua_settable(L, -3);
  }

  return 1;
}
