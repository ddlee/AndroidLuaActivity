/*
  Lua module to show Android toast
*/

#include <jni.h>
#include <android/log.h>
#include "jnicontext.h"

#ifdef __cplusplus
extern "C"
{
#endif
  #include "lua.h"
  #include "lauxlib.h"
#ifdef __cplusplus
}
#endif

#include "luatoast.h"

#ifndef LOG_TAG
#define LOG_TAG "lua"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static jclass toastClass;
static jmethodID makeTextMethod;
static jmethodID showMethod;

#define MT_NAME "toast_mt"

static jobject lua_checktoast(lua_State *L, int narg) {
  void *ud = luaL_checkudata(L, narg, MT_NAME);
  luaL_argcheck(L, *(jobject *)ud != NULL, narg, "invalid object");
  return *(jobject *)ud;
}

static int lua_toast_create(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  int duration = luaL_optint(L, 2, 0);

  JNIEnv *env = jniGetEnv();
  // ApplicationContext crashes Android emulator
  //  jobject context = jniGetApplicationContext();
  jobject context = jniGetActivity();

  jobject toastObject = 
    env->CallStaticObjectMethod(toastClass,
				makeTextMethod,
				context,
				env->NewStringUTF(text),
				duration);
  LOGI("toastObject %p", toastObject);

  jobject *jptr = (jobject *)lua_newuserdata(L, sizeof(jobject));
  *jptr = env->NewGlobalRef(toastObject);
  env->DeleteLocalRef(toastObject);

  luaL_getmetatable(L, MT_NAME);
  lua_setmetatable(L, -2);
  return 1;
}

static int lua_toast_delete(lua_State *L) {
  jobject obj = lua_checktoast(L, 1);

  JNIEnv *env = jniGetEnv();
  env->DeleteGlobalRef(obj);

  return 0;
}

static int lua_toast_show(lua_State *L) {
  jobject obj = lua_checktoast(L, 1);

  JNIEnv *env = jniGetEnv();
  env->CallVoidMethod(obj, showMethod);
  return 0;
}

static int lua_toast_tostring(lua_State *L) {
  jobject obj = lua_checktoast(L, 1);
  lua_pushfstring(L, "toast %p",
		  obj);
  return 1;
}

static const struct luaL_reg toast_functions[] = {
  {"new", lua_toast_create},
  {NULL, NULL}
};

static const struct luaL_reg toast_methods[] = {
  {"show", lua_toast_show},
  {"__gc", lua_toast_delete},
  {"__tostring", lua_toast_tostring},
  {NULL, NULL}
};

/*
static int lua_toast_newmetatable(lua_State *L, const char *tname) {
  // Setup metatable
  luaL_newmetatable(L, tname);
  
  // OO access: mt.__index methods
  lua_newtable(L);
  luaL_register(L, NULL, toast_methods);
  lua_setfield(L, -2, "__index");

  return 1;
}
*/

extern "C"
int luaopen_toast (lua_State *L) {
  LOGI("luaopen_toast called");
  
  // Uses Java VM and application context
  JNIEnv *env = jniGetEnv();

  jclass toastClassLocal = env->FindClass("android/widget/Toast");
  LOGI("toastClassLocal %p", toastClassLocal);
  if (toastClassLocal == 0) {
    LOGE("Could not find Toast class");
    return 0;
  }
  toastClass = (jclass) env->NewGlobalRef(toastClassLocal);
  LOGI("toastClass %p", toastClass);
  if (toastClass == 0) {
    LOGE("Could not make global ref");
    return 0;
  }

  makeTextMethod =
    env->GetStaticMethodID(toastClass,
			   "makeText",
			   "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
  LOGI("makeTextMethod %p", makeTextMethod);
  if (makeTextMethod == 0) {
    LOGE("Could not get makeText method");
    return 0;
  }

  showMethod =
    env->GetMethodID(toastClass,
		     "show",
		     "()V");
  LOGI("showMethod %p", showMethod);
  if (showMethod == 0) {
    LOGE("Could not get show method");
    return 0;
  }

  luaL_newmetatable(L, MT_NAME);
  // OO access: mt.__index = mt
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_register(L, NULL, toast_methods);

  luaL_register(L, "toast", toast_functions);
  return 1;
}
