/*
  Lua module to control Android Text To Speech (TTS)
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

#include "luatts.h"

#ifndef LOG_TAG
#define LOG_TAG "lua"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static jclass ttsClass;
static jclass localeClass;
static jmethodID ttsInitMethod;
static jmethodID setLanguageMethod;
static jmethodID speakMethod;
static int tts_QUEUE_ADD;
static int tts_QUEUE_FLUSH;

#define MT_NAME "tts_mt"

static jobject lua_checktts(lua_State *L, int narg) {
  void *ud = luaL_checkudata(L, narg, MT_NAME);
  luaL_argcheck(L, *(jobject *)ud != NULL, narg, "invalid object");
  return *(jobject *)ud;
}

static int lua_tts_create(lua_State *L) {
  JNIEnv *env = jniGetEnv();
  jobject appContext = jniGetApplicationContext();
  
  jobject ttsObject = env->NewObject(ttsClass, ttsInitMethod,
				     appContext, NULL);
  LOGI("ttsObject %p", ttsObject);

  jobject *jptr = (jobject *)lua_newuserdata(L, sizeof(jobject));
  *jptr = env->NewGlobalRef(ttsObject);
  env->DeleteLocalRef(ttsObject);

  luaL_getmetatable(L, MT_NAME);
  lua_setmetatable(L, -2);
  return 1;
}

static int lua_tts_delete(lua_State *L) {
  jobject obj = lua_checktts(L, 1);

  JNIEnv *env = jniGetEnv();
  env->DeleteGlobalRef(obj);

  return 0;
}

static int lua_tts_speak(lua_State *L) {
  jobject obj = lua_checktts(L, 1);
  const char *text = luaL_checkstring(L, 2);
  int mode = luaL_optint(L, 3, tts_QUEUE_ADD);

  JNIEnv *env = jniGetEnv();
  int ret = env->CallIntMethod(obj, speakMethod,
			       env->NewStringUTF(text),
			       mode,
			       NULL);
  lua_pushinteger(L, ret);
  return 1;
}

static int lua_tts_speakFlush(lua_State *L) {
  jobject obj = lua_checktts(L, 1);
  const char *text = luaL_checkstring(L, 2);

  JNIEnv *env = jniGetEnv();
  int ret = env->CallIntMethod(obj, speakMethod,
			       env->NewStringUTF(text),
			       tts_QUEUE_FLUSH,
			       NULL);
  lua_pushinteger(L, ret);
  return 1;
}

static int lua_tts_tostring(lua_State *L) {
  jobject obj = lua_checktts(L, 1);
  lua_pushfstring(L, "tts %p",
		  obj);
  return 1;
}

static const struct luaL_reg tts_functions[] = {
  {"new", lua_tts_create},
  {NULL, NULL}
};

static const struct luaL_reg tts_methods[] = {
  {"speak", lua_tts_speak},
  {"speakFlush", lua_tts_speakFlush},
  {"__gc", lua_tts_delete},
  {"__tostring", lua_tts_tostring},
  {NULL, NULL}
};

extern "C"
int luaopen_tts (lua_State *L) {
  LOGI("luaopen_tts called");
  
  // Uses Java VM and application context
  JNIEnv *env = jniGetEnv();
  jobject appContext = jniGetApplicationContext();

  jclass ttsClassLocal = env->FindClass("android/speech/tts/TextToSpeech");
  if (ttsClassLocal == 0) {
    LOGE("Could not find TextToSpeech class");
    return 0;
  }
  ttsClass = (jclass) env->NewGlobalRef(ttsClassLocal);
  LOGI("ttsClass %p", ttsClass);
  if (ttsClass == 0) {
    LOGE("Could not make global ref");
    return 0;
  }

  jfieldID queue_add_fid = env->GetStaticFieldID(ttsClass, "QUEUE_ADD", "I");
  LOGI("QUEUE_ADD fid %p", queue_add_fid);
  tts_QUEUE_ADD = env->GetStaticIntField(ttsClass, queue_add_fid);

  jfieldID queue_flush_fid = env->GetStaticFieldID(ttsClass, "QUEUE_FLUSH", "I");
  LOGI("QUEUE_FLUSH fid %p", queue_flush_fid);
  tts_QUEUE_FLUSH = env->GetStaticIntField(ttsClass, queue_flush_fid);

  ttsInitMethod =
    env->GetMethodID(ttsClass,
		     "<init>",
		     "(Landroid/content/Context;Landroid/speech/tts/TextToSpeech$OnInitListener;)V");
  LOGI("ttsInitMethod %p", ttsInitMethod);
  if (ttsInitMethod == 0) {
    LOGE("Could not get TextToSpeech constructor method");
    return 0;
  }

  speakMethod =
    env->GetMethodID(ttsClass,
		     "speak",
		     "(Ljava/lang/String;ILjava/util/HashMap;)I");
  LOGI("speakMethod %p", speakMethod);
  if (speakMethod == 0) {
    LOGE("Could not get speakText method");
    return 0;
  }

  setLanguageMethod =
    env->GetMethodID(ttsClass,
		     "setLanguage",
		     "(Ljava/util/Locale;)I");
  LOGI("setLanguageMethod %p", setLanguageMethod);
  if (setLanguageMethod == 0) {
    LOGE("Could not get setLanguage method");
    return 0;
  }

  luaL_newmetatable(L, MT_NAME);
  // OO access: mt.__index = mt
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_register(L, NULL, tts_methods);

  luaL_register(L, "tts", tts_functions);


  return 1;
}
