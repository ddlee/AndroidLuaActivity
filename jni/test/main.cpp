//BEGIN_INCLUDE(all)

#include <jni.h>
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "luajni.h"
#include "luaasset.h"
#include "luavibrator.h"
#include "luatoast.h"

#define LOG_TAG "luaNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

struct engine {
  struct android_app* app;
  lua_State *L;
};

#ifdef __cplusplus
extern "C"
#endif
int32_t inputHandler(struct android_app* app, AInputEvent* event) {
  struct engine* engine = (struct engine*)app->userData;

  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    luaL_dostring(engine->L, "inputEventTypeMotion()");

    return 1;
  }
  return 0;
}

ANativeWindow* window;
const EGLint attribs[] = {
  EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
  EGL_BLUE_SIZE, 8,
  EGL_GREEN_SIZE, 8,
  EGL_RED_SIZE, 8,
  EGL_NONE
};
EGLint w, h, dummy, format;
EGLint numConfigs;
EGLConfig config;
EGLDisplay display;
EGLSurface surface;
EGLContext context;

#ifdef __cplusplus
extern "C"
#endif
void cmdHandler(struct android_app* app, int32_t cmd) {
  struct engine* engine = (struct engine*)app->userData;

  switch (cmd) {
  case APP_CMD_DESTROY:
    LOGI("APP_CMD_DESTROY");
    lua_close(engine->L);
    break;
  case APP_CMD_START:
    LOGI("APP_CMD_START");
    luaL_dostring(engine->L, "onStart()");
    break;
  case APP_CMD_STOP:
    LOGI("APP_CMD_STOP");
    luaL_dostring(engine->L, "onStop()");
    break;
  case APP_CMD_PAUSE:
    LOGI("APP_CMD_PAUSE");
    luaL_dostring(engine->L, "onPause()");
    break;
  case APP_CMD_RESUME:
    LOGI("APP_CMD_RESUME");
    luaL_dostring(engine->L, "onResume()");
    break;
  case APP_CMD_INIT_WINDOW:
    LOGI("APP_CMD_INIT_WINDOW");

    /*
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(window, 0, 0, format);
    surface = eglCreateWindowSurface(display, config, window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);
    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
      LOGW("Unable to eglMakeCurrent");
      return;
    }
    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    // Initialize GL state.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);
    */

    break;

  case APP_CMD_TERM_WINDOW:
    LOGI("APP_CMD_TERM_WINDOW");

    /*
    if (display != EGL_NO_DISPLAY) {
      eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      if (context != EGL_NO_CONTEXT) {
	eglDestroyContext(display, context);
      }
      if (surface != EGL_NO_SURFACE) {
	eglDestroySurface(display, surface);
      }
      eglTerminate(display);
    }
    display = EGL_NO_DISPLAY;
    context = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;
    */
    
    break;
  case APP_CMD_GAINED_FOCUS:
 
    break;
  case APP_CMD_LOST_FOCUS:

    break;
  case APP_CMD_SAVE_STATE:
 
    break;
  }
}

#ifdef __cplusplus
extern "C"
#endif
void android_main(struct android_app* state) {
  struct engine engine;

  // Make sure glue isn't stripped.
  app_dummy();

  LOGI("android_main");

  memset(&engine, 0, sizeof(engine));
  state->userData = &engine;
  state->onAppCmd = cmdHandler;
  state->onInputEvent = inputHandler;

  // Dynamically load lua library
  //  void *handle = dlopen("/data/data/com.example.luanative/lib/liblua.so", RTLD_LAZY);

  engine.app = state;
  engine.L = luaL_newstate();

  luaL_openlibs(engine.L);

  jniSetJavaVM(state->activity->vm);
  jniSetContext(state->activity->clazz);
  JNIEnv* env = jniGetEnv();

  luaopen_asset(engine.L);
  luaopen_vibrator(engine.L);
  luaopen_toast(engine.L);

  luaL_doasset(engine.L, "main.lua");

  if (state->savedState != NULL) {

  }

  while (1) {
    // Read all pending events.
    int ident;
    int events;
    struct android_poll_source* source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
      
    while ((ident=ALooper_pollAll(0, NULL, &events,
				  (void**)&source)) >= 0) {

      LOGI("ident: %d, events %d", ident, events);

      // Process this event.
      if (source != NULL) {
	source->process(state, source);
      }

      // Check if we are exiting.
      if (state->destroyRequested != 0) {

	return;
      }
    }

    usleep(100000);

  }
}

//END_INCLUDE(all)
