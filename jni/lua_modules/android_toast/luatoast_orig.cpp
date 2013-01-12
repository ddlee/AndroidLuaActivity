/*
  Lua module to show Android toast
*/

#include <jni.h>
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

#include "luajni.h"
#include "luatoast.h"

#define LOG_TAG "luaToast"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static jclass toastClass;
static jmethodID makeTextMethod;
static jmethodID showMethod;

static int lua_toast_show(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  int duration = luaL_optint(L, 2, 0);
  
  JNIEnv *env = jniGetEnv();
  jobject appContext = jniGetApplicationContext();
  
  jobject toastObject = 
    env->CallStaticObjectMethod(toastClass,
				makeTextMethod,
				appContext,
				env->NewStringUTF(text),
				duration);
  LOGI("toastObject %p", toastObject);
  if (toastObject == 0) {
    LOGE("Could not get toast object");
    return 0;
  }
  
  env->CallVoidMethod(toastObject, showMethod);
  return 0;
}

static const struct luaL_reg toast_lib[] = {
  {"show", lua_toast_show},
  {NULL, NULL}
};

LUALIB_API
int luaopen_toast (lua_State *L) {
  LOGI("luaopen_toast called");
  
  // Uses Java VM and application context
  JNIEnv *env = jniGetEnv();
  jobject appContext = jniGetApplicationContext();

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

  luaL_register(L, "toast", toast_lib);
  return 1;
}
