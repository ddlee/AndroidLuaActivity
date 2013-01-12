#ifndef activity_h
#define activity_h

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <android/native_activity.h>
#include <android/configuration.h>
#include <android/looper.h>
#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG "lua"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

struct engine {
  ANativeActivity *activity;
  lua_State *L;

  AConfiguration *config;
  ANativeWindow *window;

  AInputQueue *queue;
  ALooper *looper;

  int msgread;
  int msgwrite;
};


#ifdef __cplusplus
}
#endif

#endif
