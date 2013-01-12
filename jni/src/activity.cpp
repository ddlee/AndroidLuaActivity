#include <jni.h>
#include <string.h>
#include <unistd.h>

#include <android/native_activity.h>
#include <android/looper.h>

#include "activity.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "jnicontext.h"

// Lua callback function of name
static int lua_callback_errchk(lua_State *L, const char *name, int nargs) {
  lua_getfield(L, LUA_GLOBALSINDEX, name);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    return 0;
  }
  // Move lua function before pushed arguments
  lua_insert(L, -(nargs+1));
  if (lua_pcall(L, nargs, LUA_MULTRET, 0)) {
    LOGE("pcall %s", lua_tostring(L, -1));
    lua_pop(L, 1);
    return 0;
  }
  return lua_gettop(L);
}

// Main thread callback for input queue events
static int onInputEvent(int fd, int events, void *data) {
  struct engine* engine = (struct engine *)data;
  AInputEvent* event = NULL;
  if (AInputQueue_getEvent(engine->queue, &event) >= 0) {
    //    LOGI("Input event: type=%d", AInputEvent_getType(event));
    int handled = AInputQueue_preDispatchEvent(engine->queue, event);
    if (handled) {
      return 1;
    }

    lua_pushlightuserdata(engine->L, event);
    handled = 0;
    if (lua_callback_errchk(engine->L, "onInputEvent", 1)) {
      handled = luaL_optint(engine->L, -1, 0);
      lua_pop(engine->L, 1);
    }
    // handled = 0 to allow default processing (Back Key->finish)
    AInputQueue_finishEvent(engine->queue, event, handled);
  }

  // Return 1 to allow additional callbacks
  return 1;
}

/// Lua function to post string to main thread looper pipe
/// Requires engine pointer to be stored as upvalue
static int lua_uipost(lua_State *L) {
  size_t len;
  const char *str = luaL_checklstring(L, 1, &len);
  struct engine *engine =
    (struct engine *)lua_touserdata(L, lua_upvalueindex(1));
  if (engine == NULL) return luaL_error(L, "invalid upvalue");
  int ret = write(engine->msgwrite, str, len);
  lua_pushinteger(L, ret);
  return 1;
}

// Main thread looper callback function for executing uipost message on lua
#define CMD_BUFFER_SIZE 4096
static int uipostCallback(int fd, int events, void *data) {
  struct engine* engine = (struct engine *)data;
  char str[CMD_BUFFER_SIZE];
  int nread = read(engine->msgread, str, CMD_BUFFER_SIZE-1);
  if (nread > 0) {
    // Ensure string is null terminated
    str[nread] = '\0';
    //    LOGI("uipost: %s", str);
  
    if (luaL_loadstring(engine->L, str)) {
      LOGE("uipost loadstring: %s", lua_tostring(engine->L, -1));
      lua_pop(engine->L, 1);
      return 1;
    }

    if (lua_pcall(engine->L, 0, 0, 0)) {
      LOGE("uipost pcall %s", lua_tostring(engine->L, -1));
      lua_pop(engine->L, 1);
      return 1;
    }
  }
  else {
    LOGI("No data from looper");
  }
  
  // Return 1 to allow additional callbacks
  return 1;
}

static void onDestroy(ANativeActivity* activity) {
  LOGI("onDestroy: %p", activity);
  //  callSuperVoidMethod(activity->env, activity->clazz, "onDestroy");

  struct engine* engine = (struct engine *)activity->instance;
  lua_callback_errchk(engine->L, "onDestroy", 0);
  lua_close(engine->L);
  free(engine);
}

static void onStart(ANativeActivity* activity) {
  LOGI("onStart: %p", activity);

  struct engine* engine = (struct engine *)activity->instance;
  lua_callback_errchk(engine->L, "onStart", 0);
}

static void onStop(ANativeActivity* activity) {
  LOGI("onStop: %p", activity);

  struct engine* engine = (struct engine *)activity->instance;
  lua_callback_errchk(engine->L, "onStop", 0);
}

static void onResume(ANativeActivity* activity) {
  LOGI("onResume: %p", activity);

  struct engine* engine = (struct engine *)activity->instance; 
  lua_callback_errchk(engine->L, "onResume", 0);
}

static void onPause(ANativeActivity* activity) {
  LOGI("onPause: %p", activity);

  struct engine* engine = (struct engine *)activity->instance;
  lua_callback_errchk(engine->L, "onPause", 0);
}

static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
  LOGI("onSaveInstanceState: %p", activity);
  void* savedState = NULL;
  *outLen = 0;

  return savedState;
}

static void onConfigurationChanged(ANativeActivity* activity) {
  LOGI("onConfigurationChanged: %p", activity);

}

static void onLowMemory(ANativeActivity* activity) {
  LOGI("onLowMemory: %p", activity);

}

static void onWindowFocusChanged(ANativeActivity* activity, int focused) {
  LOGI("onWindowFocusChanged: %p", activity);

}

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
  LOGI("onNativeWindowCreated: %p", activity);
  struct engine* engine = (struct engine *)activity->instance;
  engine->window = window;

  lua_pushlightuserdata(engine->L, window);
  lua_callback_errchk(engine->L, "onNativeWindowCreated", 1);
}

static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
  LOGI("onNativeWindowDestroyed: %p", activity);
  struct engine* engine = (struct engine *)activity->instance;
  engine->window = NULL;

  lua_pushlightuserdata(engine->L, window);
  lua_callback_errchk(engine->L, "onNativeWindowDestroyed", 1);
}

static void onInputQueueCreated(ANativeActivity* activity, AInputQueue *queue) {
  LOGI("onInputQueueCreated: %p", activity);
  struct engine* engine = (struct engine *)activity->instance;

  AInputQueue_attachLooper(queue, engine->looper,
			   ALOOPER_POLL_CALLBACK, onInputEvent, engine);
  engine->queue = queue;
}

static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue *queue) {
  LOGI("onInputQueueDestroyed: %p", activity);
  struct engine* engine = (struct engine *)activity->instance;

  AInputQueue_detachLooper(queue);
  engine->queue = NULL;
}

// uipost to push string onto main looper
// Then uipostCallback will execute string in main thread lua

extern "C"
void ANativeActivity_onCreate(ANativeActivity* activity,
			      void* savedState, size_t savedStateSize) {
  LOGI("NativeActivity onCreate: %p", activity);
  LOGI("Internal datapath: %s", activity->internalDataPath);
  LOGI("External datapath: %s", activity->externalDataPath);

  // Setup callbacks
  activity->callbacks->onDestroy = onDestroy;
  activity->callbacks->onStart = onStart;
  activity->callbacks->onStop = onStop;
  activity->callbacks->onResume = onResume;
  activity->callbacks->onPause = onPause;
  activity->callbacks->onSaveInstanceState = onSaveInstanceState;
  activity->callbacks->onConfigurationChanged = onConfigurationChanged;
  activity->callbacks->onLowMemory = onLowMemory;
  activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
  activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
  activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
  activity->callbacks->onInputQueueCreated = onInputQueueCreated;
  activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;

  // Construct global engine struct with activity, looper and lua info
  struct engine* engine = (struct engine*)malloc(sizeof(struct engine));
  memset(engine, 0, sizeof(struct engine));
  engine->activity = activity;
  activity->instance = engine;

  // Setup command pipe
  int msgpipe[2];
  if (pipe(msgpipe)) {
    LOGI("Could not create pipe");
  }
  engine->msgread = msgpipe[0];
  engine->msgwrite = msgpipe[1];

  // Add pipe to main looper
  engine->looper = ALooper_forThread();
  LOGI("main looper: %p", engine->looper);
  ALooper_addFd(engine->looper, engine->msgread, ALOOPER_POLL_CALLBACK,
		ALOOPER_EVENT_INPUT, uipostCallback, engine);

  // Set Java context
  jniSetJavaVM(activity->vm);
  jniSetContext(activity->clazz);
  //  JNIEnv* env = jniGetEnv();

  // Bug in Android 2.3 which returns null for internalDataPath
  if (activity->internalDataPath == NULL) {
    activity->internalDataPath = jniGetFilesDir(activity);
    LOGI("Activity internalDataPath set: %s", activity->internalDataPath);
  }

  // Set LUA_CPATH environment for lua require() to load dynamic libraries
  // CPATH needs to be constructed from activity directory information
#define MAX_STRING_LENGTH 256
  char dirname[MAX_STRING_LENGTH+1];
  strncpy(dirname, activity->internalDataPath, MAX_STRING_LENGTH);
  char *ifind = strstr(dirname, "/files");
  if (ifind) *ifind = '\0'; // Truncate "/files" from dirname
  strncat(dirname, "/lib/lib?.so;", MAX_STRING_LENGTH-strlen(dirname));
  setenv(LUA_CPATH, dirname, 1);
  /* Alternative code to set package.cpath
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "cpath"); // package.cpath on top of stack
  strncat(dirname, lua_tostring(L, -1), MAX_STRING_LENGTH-strlen(dirname));
  lua_pop(L, 1); // get rid of package.cpath
  lua_pushstring(L, dirname); // push new cpath
  lua_setfield(L, -2, "cpath"); // set package.cpath
  lua_pop(L, 1); // get rid of package
  */
LOGI("cpath: %s", dirname);

  // New lua state
  lua_State *L = luaL_newstate();
  engine->L = L;
  luaL_openlibs(L);

  // Register post() C closure function
  lua_pushlightuserdata(L, engine);
  lua_pushcclosure(L, lua_uipost, 1);
  lua_setglobal(L, "uipost");

  // Open lua asset module and start init.lua
  if (luaL_dostring(L, "require('asset'); asset.dofile('init.lua')")) {
      LOGE("init.lua: %s", lua_tostring(L, -1));
      lua_pop(L, 1);
  }

  int narg = 0;
  if (savedState) {
    lua_pushlstring(L, (const char *)savedState, savedStateSize);
    narg = 1;
  }
  lua_callback_errchk(L, "onCreate", narg);
}

/*
extern "C"
void ANativeActivity_finish(ANativeActivity *activity) {
  LOGI("Finish: %p\n", activity);
  struct engine* engine = (struct engine *)activity->instance;
  lua_close(engine->L);
}
*/
