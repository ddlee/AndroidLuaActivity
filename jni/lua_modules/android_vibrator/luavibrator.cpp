/*
  Lua module to activate Android vibrator
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

#include "luavibrator.h"

#ifndef LOG_TAG
#define LOG_TAG "lua"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static jobject vibratorObject;
static jmethodID cancelMethod;
static jmethodID hasVibratorMethod;
static jmethodID vibrateOnMethod;
static jmethodID vibratePatternMethod;

static int lua_vibrator_hasvibrator(lua_State *L) {
  if (vibratorObject == 0) {
    LOGE("Invalid vibrator object");
    return 0;
  }
  if (hasVibratorMethod == 0) {
    LOGE("Invalid vibrator method");
    return 0;
  }
  JNIEnv *env = jniGetEnv();

  int ret = (int) env->CallBooleanMethod(vibratorObject,
					 hasVibratorMethod);
  lua_pushboolean(L, ret);
  return 1;
}

static int lua_vibrator_cancel(lua_State *L) {
  if (vibratorObject == 0) {
    LOGE("Invalid vibrator object");
    return 0;
  }
  JNIEnv *env = jniGetEnv();

  env->CallVoidMethod(vibratorObject, cancelMethod);

  if (env->ExceptionOccurred()) {
    LOGE("Could not call cancel method");
    return 0;
  }

  return 0;
}

static int lua_vibrator_vibrate(lua_State *L) {
  if (vibratorObject == 0) {
    LOGE("Invalid vibrator object");
    return 0;
  }
  JNIEnv *env = jniGetEnv();

  int narg = lua_gettop(L);
  if (narg < 1) return 0;

  if (lua_istable(L, 1)) {
    // Construct JNI long array
    int npattern = lua_objlen(L, 1);
    jlong t[npattern];
    for (int i = 0; i < npattern; i++) {
      lua_rawgeti(L, 1, i);
      t[i] = lua_tointeger(L, -1);
      lua_pop(L, 1);
    }
    jlongArray pattern = env->NewLongArray(npattern);
    env->SetLongArrayRegion(pattern, 0, npattern, t);

    // Get repeat index (-1 for no repeat by default)
    jint repeat = luaL_optint(L, 2, -1);
    env->CallVoidMethod(vibratorObject, vibratePatternMethod,
			pattern, repeat);
  }
  else {
    jlong msec = luaL_checkint(L, 1);
    env->CallVoidMethod(vibratorObject, vibrateOnMethod, msec);
  }

  if (env->ExceptionOccurred()) {
    LOGE("Could not call vibrate method");
    return 0;
  }

  return 0;
}

static const struct luaL_reg vibrator_lib[] = {
  {"cancel", lua_vibrator_cancel},
  {"vibrate", lua_vibrator_vibrate},
  {NULL, NULL}
};

#ifdef __cplusplus
extern "C"
#endif
int luaopen_vibrator (lua_State *L) {
  LOGI("luaopen_vibrator called");

  JNIEnv *env = jniGetEnv();
  jobject context = jniGetActivity();

  // #include <binder/IServiceManager.h>
  // sp<IServiceManager> serviceManager = defaultServiceManager();
  jmethodID methodGetSystemService =
    env->GetMethodID(env->GetObjectClass(context),
		     "getSystemService",
		     "(Ljava/lang/String;)Ljava/lang/Object;"); 
  jobject jobj = env->CallObjectMethod(context,
				       methodGetSystemService,
				       env->NewStringUTF("vibrator"));
  if (jobj == 0) {
    LOGE("Could not get vibrator object");
    return 0;
  }
  vibratorObject = env->NewGlobalRef(jobj);
  env->DeleteLocalRef(jobj);

  jclass vibratorClass = env->GetObjectClass(vibratorObject);
  LOGI("object %p, class %p", vibratorObject, vibratorClass);

  cancelMethod = env->GetMethodID(vibratorClass,
				  "cancel", "()V"); 
  vibrateOnMethod = env->GetMethodID(vibratorClass,
				     "vibrate", "(J)V"); 
  vibratePatternMethod = env->GetMethodID(vibratorClass,
					  "vibrate", "([JI)V"); 
  hasVibratorMethod = env->GetMethodID(vibratorClass,
				       "hasVibrator", "()Z"); 
  if (env->ExceptionOccurred()) {
    // hasVibrator() doesn't exist on <= Android 2.3.4
    env->ExceptionClear();
    LOGE("Could not get hasVibrator method");
  }
  /*
  else {
    jboolean hasVibrator =
      env->CallBooleanMethod(vibratorObject, hasVibratorMethod);
    if (!hasVibrator) {
      luaL_register(L, "vibrator", vibrator_lib_dummy);
      return 1;
    }
  }
  */

  luaL_register(L, "vibrator", vibrator_lib);
  return 1;
}
