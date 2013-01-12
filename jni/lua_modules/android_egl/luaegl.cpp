/*
  Lua module binding EGL functions
*/

#include <stdio.h>
#include <string.h>
#include <EGL/egl.h>
#include <android/native_window.h>
#include <android/log.h>

#ifdef __cplusplus
extern "C"
{
#endif
  #include <lua.h>
  #include <lauxlib.h>
#ifdef __cplusplus
}
#endif

#include "luaegl.h"

#define MT_NAME "egl"

#ifndef LOG_TAG
#define LOG_TAG "lua"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


class EGLClass {
public:
  EGLDisplay display;
  EGLint major, minor;
  EGLSurface surface;
  EGLContext context;
  
  EGLint num_config, config_size;
  EGLConfig* configs;
  
  // constructor
  EGLClass(EGLNativeDisplayType id = EGL_DEFAULT_DISPLAY):
    display(0), surface(0), context(0) {
    display = eglGetDisplay(id);
    if (display == EGL_NO_DISPLAY) {
      LOGE("eglGetDisplay");
    }
    
    if (!eglInitialize(display, &major, &minor)) {
      LOGE("eglInitialize: %p", display);
    }
    LOGI("EGL: display=%p, major=%d, minor=%d", display, major, minor);

    // First get number of configurations
    eglGetConfigs(display, NULL, 0, &config_size);

    // Allocate configs array
    configs = new EGLConfig[config_size];
    // Get EGL configs
    if (!eglGetConfigs(display, configs, config_size, &num_config)) {
      LOGE("eglGetConfigs");
    }
  }

  // destructor
  virtual ~EGLClass() {
    if (configs) delete[] configs;

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
  }

  bool ChooseConfig(EGLint const *attrib_list) {
    return eglChooseConfig(display, attrib_list,
			   configs, config_size, &num_config);
  }

  // By default use first configuration
  bool CreateSurface(ANativeWindow *window, int nconfig) {
    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    EGLint format;
    eglGetConfigAttrib(display, configs[nconfig],
		       EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(window, 0, 0, format);

    surface = eglCreateWindowSurface(display, configs[nconfig],
				     window, NULL);
    if (surface == EGL_NO_SURFACE) {
      LOGE("eglCreateWindowSurface");
      return false;
    }

    context = eglCreateContext(display, configs[nconfig],
			       EGL_NO_CONTEXT, NULL);
    if (context == EGL_NO_CONTEXT) {
      LOGE("eglCreateContext");
      return false;
    }

    return (eglMakeCurrent(display, surface, surface, context));
  }

  bool DestroySurface() {
    return eglDestroySurface(display, surface);
  }

  bool MakeCurrent() {
    return (eglMakeCurrent(display, surface, surface, context));
  }

  bool SwapBuffers() {
    return (eglSwapBuffers(display, surface));
  }
};

typedef struct luaIntConst {
  const char *key;
  int value;
} luaIntConst;

static luaIntConst eglConfigAttributes[] = {
  { "ALPHA_SIZE", EGL_ALPHA_SIZE},
  { "BLUE_SIZE", EGL_BLUE_SIZE},
  { "BUFFER_SIZE", EGL_BUFFER_SIZE},
  { "CONFIG_ID", EGL_CONFIG_ID},
  { "DEPTH_SIZE", EGL_DEPTH_SIZE},
  { "GREEN_SIZE", EGL_GREEN_SIZE},
  { "LEVEL", EGL_LEVEL},
  { "MAX_PBUFFER_HEIGHT", EGL_MAX_PBUFFER_HEIGHT},
  { "MAX_PBUFFER_WIDTH", EGL_MAX_PBUFFER_WIDTH},
  { "LEVEL", EGL_LEVEL},
  { "NATIVE_RENDERABLE", EGL_NATIVE_RENDERABLE},
  { "NATIVE_VISUAL_ID", EGL_NATIVE_VISUAL_ID},
  { "RED_SIZE", EGL_RED_SIZE},
  { "STENCIL_SIZE", EGL_STENCIL_SIZE},
  { "SURFACE_TYPE", EGL_SURFACE_TYPE},
  { NULL, 0}
};

static luaIntConst eglSurfaceAttributes[] = {
  { "CONFIG_ID", EGL_CONFIG_ID},
  { "HEIGHT", EGL_HEIGHT},
  { "TEXTURE_FORMAT", EGL_TEXTURE_FORMAT},
  { "VERTICAL_RESOLUTION", EGL_VERTICAL_RESOLUTION},
  { "WIDTH", EGL_WIDTH},
  { NULL, 0}
};

static luaIntConst eglContextAttributes[] = {
  { "CONFIG_ID", EGL_CONFIG_ID},
  { "RENDER_BUFFER", EGL_RENDER_BUFFER},
  { NULL, 0}
};
  
static luaIntConst eglIntConst[] = {
  { "CONFORMANT", EGL_CONFORMANT},
  { "DONT_CARE", EGL_DONT_CARE},
  { "FALSE", EGL_FALSE},
  { "NONE", EGL_NONE},
  { "OPENGL_BIT", EGL_OPENGL_BIT},
  { "OPENGL_ES_BIT", EGL_OPENGL_ES_BIT},
  { "OPENGL_ES2_BIT", EGL_OPENGL_ES2_BIT},
  { "WINDOW_BIT", EGL_WINDOW_BIT},
  { "TRUE", EGL_TRUE},
  { NULL, 0}
};

static EGLClass* lua_checkeglclass(lua_State *L, int narg) {
  void *ud = luaL_checkudata(L, narg, MT_NAME);
  luaL_argcheck(L, *(EGLClass **)ud != NULL, narg, "invalid object");
  return *(EGLClass **)ud;
}

static int lua_egl_new(lua_State *L) {
  EGLNativeDisplayType id = EGL_DEFAULT_DISPLAY;
  EGLClass **ud =
    (EGLClass **)lua_newuserdata(L, sizeof(EGLClass *));
  *ud = new EGLClass(id);
  luaL_getmetatable(L, MT_NAME);
  lua_setmetatable(L, -2);
  return 1;
}

static int lua_egl_GetConfigs(lua_State *L) {
  EGLClass *egl = lua_checkeglclass(L, 1);

  lua_createtable(L, egl->config_size, 0);
  for (int i = 0; i < egl->num_config; i++) {
    // Config table
    lua_createtable(L, 0, sizeof(eglConfigAttributes)/sizeof(luaIntConst));

    // Iterate through eglConfigAttributes keys
    luaIntConst *p = eglConfigAttributes;
    EGLint attribValue;
    for (; p->key; p++) {
      if (!eglGetConfigAttrib(egl->display, egl->configs[i],
			      p->value, &attribValue)) {
	LOGE("eglGetConfigAttrib: %s", p->key);
      }
      lua_pushstring(L, p->key);
      lua_pushinteger(L, attribValue);
      lua_settable(L, -3);
    }

    // Set index of config table
    lua_rawseti(L, -2, i+1);
  }
  return 1;
}

static int lua_egl_ChooseConfig(lua_State *L) {
  EGLClass *egl = lua_checkeglclass(L, 1);
  if (!lua_istable(L, 2)) {
    luaL_error(L, "Need table argument");
  }
  // Count number of pairs in table
  lua_pushnil(L);
  int ntable = 0;
  while (lua_next(L, 2)) {
    ntable++;
    lua_pop(L, 1);
  }

  // Construct attribute list array
  LOGI("ChooseConfig: attrib_list[%d]",2*ntable+1);
  EGLint attrib_list[2*ntable+1];
  int k = 0;
  lua_pushnil(L);
  while (lua_next(L, 2)) {
    // Check if lua table key is string
    if (lua_isstring(L, -2)) {
      int attribValue = lua_tointeger(L, -1);
      // Replicate table key for tostring
      lua_pushvalue(L, -2);
      const char *attribKey = lua_tostring(L, -1);
      lua_pop(L, 1);

      // Iterate through eglConfigAttributes for matching key
      luaIntConst *c = eglConfigAttributes;
      while (c->key != NULL) {
	if (strcasecmp(c->key, attribKey) == 0) {
	  // Matching key
	  attrib_list[k++] = c->value;
	  attrib_list[k++] = attribValue;
	  LOGI("ChooseConfig attrib: %s[%d],%d", c->key, c->value, attribValue);
	  break;
	}
	c++;
      }
    } //if (lua_isstring())
    lua_pop(L, 1); // Lua table value
  } // while
  attrib_list[k++] = EGL_NONE; // Terminate attribute list

  bool ret = egl->ChooseConfig(attrib_list);
  lua_pushboolean(L, ret);
  return 1;
}

static int lua_egl_CreateSurface(lua_State *L) {
  EGLClass *egl = lua_checkeglclass(L, 1);
  if (!lua_islightuserdata(L, 2)) {
    luaL_error(L, "Argument is not light userdata");
  }
  ANativeWindow *win = (ANativeWindow *) lua_topointer(L, 2);
  int nconfig = luaL_optint(L, 3, 0);

  lua_pushboolean(L, egl->CreateSurface(win, nconfig));
  return 1;
}

static int lua_egl_DestroySurface(lua_State *L) {
  EGLClass *egl = lua_checkeglclass(L, 1);
  egl->DestroySurface();
  return 0;
}

static int lua_egl_QuerySurface(lua_State *L) {
  EGLClass *egl = lua_checkeglclass(L, 1);
  EGLint w, h;

  eglQuerySurface(egl->display, egl->surface, EGL_WIDTH, &w);
  eglQuerySurface(egl->display, egl->surface, EGL_HEIGHT, &h);

  lua_pushinteger(L, w);
  lua_pushinteger(L, h);
  
  return 2;
}

static int lua_egl_QuerySurfaceSize(lua_State *L) {
  EGLClass *egl = lua_checkeglclass(L, 1);
  EGLint w, h;

  eglQuerySurface(egl->display, egl->surface, EGL_WIDTH, &w);
  eglQuerySurface(egl->display, egl->surface, EGL_HEIGHT, &h);

  lua_pushinteger(L, w);
  lua_pushinteger(L, h);
  
  return 2;
}

static int lua_egl_MakeCurrent(lua_State *L) {
  EGLClass *egl = lua_checkeglclass(L, 1);
  lua_pushboolean(L, egl->MakeCurrent());
  return 1;
}


static int lua_egl_SwapBuffers(lua_State *L) {
  EGLClass *egl = lua_checkeglclass(L, 1);
  lua_pushboolean(L, egl->SwapBuffers());
  return 1;
}

static int lua_egl_delete(lua_State *L) {
  EGLClass *egl = lua_checkeglclass(L, 1);
  delete egl;
  return 0;
}

static int lua_egl_tostring(lua_State *L) {
  EGLClass *egl = lua_checkeglclass(L, 1);
  lua_pushfstring(L, "EGL: display %p", egl->display);
  return 1;
}

static const struct luaL_reg egl_functions[] = {
  {"new", lua_egl_new},
  {NULL, NULL}
};

static const struct luaL_reg egl_methods[] = {
  {"GetConfigs", lua_egl_GetConfigs},
  {"ChooseConfig", lua_egl_ChooseConfig},
  {"CreateSurface", lua_egl_CreateSurface},
  {"DestroySurface", lua_egl_DestroySurface},
  {"QuerySurface", lua_egl_QuerySurface},
  {"QuerySurfaceSize", lua_egl_QuerySurfaceSize},
  {"MakeCurrent", lua_egl_MakeCurrent},
  {"SwapBuffers", lua_egl_SwapBuffers},
  {"__gc", lua_egl_delete},
  {"__tostring", lua_egl_tostring},
  {NULL, NULL}
};

#ifdef __cplusplus
extern "C"
#endif
int luaopen_egl (lua_State *L) {
  luaL_register(L, "egl", egl_functions);

  // Register EGL constants
  luaIntConst *c = eglIntConst;
  for (; c->key; c++) {
    lua_pushstring(L, c->key);
    lua_pushinteger(L, c->value);
    lua_settable(L, -3);
  }

  luaL_newmetatable(L, MT_NAME);
  // OO access: mt.__index = mt
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_register(L, NULL, egl_methods);

  luaL_register(L, "egl", egl_functions);
  return 1;
}
