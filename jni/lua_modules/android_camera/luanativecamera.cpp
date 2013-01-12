/*
  Lua native module to interface to Android camera
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <android/log.h>

#ifdef __cplusplus
extern "C"
{
#endif
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
#ifdef __cplusplus
}
#endif

//#include <camera/CameraHardwareInterface.h>
#include <camera/Camera.h>
#include <camera/CameraParameters.h>

using namespace android;

#define MT_NAME "nativecamera"

#ifndef LOG_TAG
#define LOG_TAG "lua"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static int LuaCallback(lua_State *L, int ref, int nargs) {
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    return 0;
  }
  // Move lua function before pushed arguments
  lua_insert(L, -(nargs+1));
  if (lua_pcall(L, nargs, 0, 0)) {
    LOGE("callback %s", lua_tostring(L, -1));
    lua_pop(L, 1);
    return 0;
  }
  return 0;
}

class CameraClass : public CameraListener {
public:
  sp<Camera> camera;
  CameraParameters parameters;  
  int id;

  lua_State *notifyL;
  int notifyRef;
  lua_State *dataL;
  int dataRef;

  int nframe;
  unsigned char *buf;
  size_t datalen;
  size_t bufsize;

  CameraClass():id(0),
		notifyL(NULL), notifyRef(0),
		dataL(NULL), dataRef(0),
		nframe(0), buf(NULL), bufsize(0) {
    camera = NULL;
  }

  virtual ~CameraClass() {
    LOGI("CameraClass destructor");
    if (notifyRef) {
      luaL_unref(notifyL, LUA_REGISTRYINDEX, notifyRef);
    }
    if (dataRef) {
      luaL_unref(dataL, LUA_REGISTRYINDEX, dataRef);
    }
    if (camera != NULL) {
      camera.clear();
      camera = NULL;
    }
  }

  // Note: Android calls these callbacks from a separate thread!
  virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2) {
    LOGI("notify: %d %d %d", msgType, ext1, ext2);
    //if (msgType & CAMERA_MSG_FOCUS) LOGI("notify focus");

    if (notifyRef) {
      int nargs = 3;
      lua_pushinteger(notifyL, msgType);
      lua_pushinteger(notifyL, ext1);
      lua_pushinteger(notifyL, ext2);
      LuaCallback(notifyL, notifyRef, nargs);
    }
    return;
  }

  virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr) {
#if 1
    LOGI("postData msg %d, buffer:%p+%d(%d bytes)", msgType,
	 dataPtr->pointer(), dataPtr->offset(), dataPtr->size());
#endif

    nframe++;

    datalen = dataPtr->size();
    // Cannot just store pointer, since IMemory pointer is invalid
    // outside of postData() callback:
    // buf = (unsigned char *)dataPtr->pointer()+dataPtr->offset();

    // Need to copy data buffer if pointer is to be used in another thread
    if (bufsize < datalen) {
      bufsize = datalen;
      buf = (unsigned char *) realloc(buf, bufsize);
    }
    if (buf) {
      memcpy(buf, (unsigned char *)dataPtr->pointer()+dataPtr->offset(), datalen);
    }

    if (dataRef) {
      int nargs = 2;
      lua_pushlightuserdata(dataL, buf);
      lua_pushinteger(dataL, datalen);
      LuaCallback(dataL, dataRef, nargs);
    }
    return;
  }
  virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType,
				 const sp<IMemory>& dataPtr) {
    LOGI("postDataTimestamp: %d %d", timestamp, msgType);
    if (dataPtr != NULL) {
      nframe++;
      datalen = dataPtr->size();
      buf = (unsigned char *)dataPtr->pointer();
      LOGI("callback postDataTimestamp: %d %p(%d bytes)", msgType, buf, datalen);
    }
    camera->releaseRecordingFrame(dataPtr);
  }

  void connect(int _id) {
    LOGI("camera connecting to id %d",_id);
    camera = 0;
    camera = Camera::connect(_id);
    id = _id;
  }
  void disconnect() {
    camera->disconnect();
  }
};

static CameraClass* lua_checkcameraclass(lua_State *L, int narg) {
  void *ud = luaL_checkudata(L, narg, MT_NAME);
  luaL_argcheck(L, *(CameraClass **)ud != NULL, narg, "invalid object");
  return *(CameraClass **)ud;
}

static int lua_camera_getNumberOfCameras(lua_State *L) {
  int n = Camera::getNumberOfCameras();
  lua_pushinteger(L, n);
  return 1;
}

static int lua_camera_new(lua_State *L) {
  int id = luaL_optint(L, 1, 0);
  CameraClass **ud =
    (CameraClass **)lua_newuserdata(L, sizeof(CameraClass *));
  *ud = new CameraClass();
  luaL_getmetatable(L, MT_NAME);
  lua_setmetatable(L, -2);
  return 1;
}

static int lua_camera_connect(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  int id = luaL_optint(L, 2, 0);
  cam->connect(id);
  return 0;
}

static int lua_camera_disconnect(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  cam->camera->disconnect();
  return 0;
}
  
static int lua_camera_getParameters(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  String8 pString8 = cam->camera->getParameters();
  CameraParameters parameters(pString8);
  //  parameters.unflatten(pString8);

  const char* pstr = pString8.string();
  LOGI("getParameters: %s", pstr);

  lua_pushstring(L, pstr);
  return 1;
}

static int lua_camera_setParameters(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  String8 pString8 = cam->camera->getParameters();
  CameraParameters parameters(pString8);

  int narg = lua_gettop(L);
  for (int i = 2; i < narg; i+=2) {
    const char *key = luaL_checkstring(L, i);
    const char *value = luaL_checkstring(L, i+1);
    parameters.set(key, value);
  }

  int status = cam->camera->setParameters(parameters.flatten());
  lua_pushinteger(L, status);
  return 1;
}

static int lua_camera_setNotifyCallback(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  if (cam->notifyRef) {
    luaL_unref(L, LUA_REGISTRYINDEX, cam->notifyRef);
    cam->notifyRef = 0;
  }
  if (lua_isfunction(L, 2)) {
    cam->notifyL = L;
    cam->notifyRef = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  lua_pushinteger(L, cam->notifyRef);
  return 1;
}

static int lua_camera_setDataCallback(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  if (cam->dataRef) {
    luaL_unref(L, LUA_REGISTRYINDEX, cam->dataRef);
    cam->dataRef = 0;
  }
  if (lua_isfunction(L, 2)) {
    cam->dataL = L;
    cam->dataRef = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  lua_pushinteger(L, cam->dataRef);
  return 1;
}

static int lua_camera_setListener(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  cam->camera->setListener(cam);
  return 0;
}

static int lua_camera_setPreviewCallbackFlags(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  // Preview callback with copy by default
  int flags = luaL_optint(L, 1,
			  FRAME_CALLBACK_FLAG_ENABLE_MASK |
			  FRAME_CALLBACK_FLAG_COPY_OUT_MASK);
  cam->camera->setPreviewCallbackFlags(flags);
  return 0;
}


//#include <surfaceflinger/Surface.h>
// Hack Surface class to pass null pointer in setPreviewDisplay
class android::Surface : public RefBase { };
static int lua_camera_setPreviewDisplay(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  /*
  sp<Surface> surface = NULL;
  cam->camera->setPreviewDisplay(surface);
  */
  cam->camera->setPreviewDisplay((const sp<Surface> &)(NULL));
  return 0;
}

// This can only be used with Android 3.0+ (platform-10)
class android::ISurfaceTexture : public RefBase { };
static int lua_camera_setPreviewTexture(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  int texName = luaL_optint(L, 2, 0);
  // Need to create SurfaceTexture with texture id 0 first!
  cam->camera->setPreviewTexture((const sp<ISurfaceTexture> &)(NULL));
  return 0;
}

static int lua_camera_startPreview(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  LOGI("startPreview");
  // Preview with copy out needed:
  cam->camera->setPreviewCallbackFlags(FRAME_CALLBACK_FLAG_ENABLE_MASK |
				       FRAME_CALLBACK_FLAG_COPY_OUT_MASK);
  // Setup listener callbacks
  cam->camera->setListener(cam);

  int status = cam->camera->startPreview();
  lua_pushinteger(L, status);
  return 1;
}

static int lua_camera_stopPreview(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  cam->camera->stopPreview();
  return 0;
}

static int lua_camera_startRecording(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  int status = cam->camera->startRecording();
  lua_pushinteger(L, status);
  return 1;
}

static int lua_camera_stopRecording(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  LOGI("stopRecording");
  cam->camera->stopRecording();
  return 0;
}

static int lua_camera_delete(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  delete cam;
  return 0;
}

static int lua_camera_tostring(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  lua_pushfstring(L, "Camera(%p): id %d, nframe %d",
		  cam,
		  cam->id,
		  cam->nframe);
  return 1;
}

static int lua_camera_yuv420torgba(lua_State *L) {
  static int *rgba = NULL;
  static int rgbalen = 0;

  if (!lua_islightuserdata(L, 1)) {
    return luaL_error(L, "Need yuv420 pointer");
  }
  unsigned char* yuv = (unsigned char *)lua_touserdata(L, 1);
  int len = luaL_checkinteger(L, 2);

  int nfactor = sqrt(len/18);
  int width = 4*nfactor;
  int height = 3*nfactor;

  if (rgbalen < width*height) {
    rgbalen = width*height;
    rgba = (int *) realloc(rgba, rgbalen*sizeof(int));
    if (rgba == NULL) {
      return luaL_error(L, "Could not allocate RGBA memory");
    }
  }

  for (int j = 0, yp = 0; j < height; j++) {
    int uvp = width*height + (j >> 1) * width, u = 0, v = 0;
    for (int i = 0; i < width; i++, yp++) {
      int y = (0xff & ((int) yuv[yp])) - 16;
      if (y < 0) y = 0;
      if ((i & 1) == 0) {
	v = (0xff & yuv[uvp++]) - 128;
	u = (0xff & yuv[uvp++]) - 128;
      }

      int y1192 = 1192*y;
      int r = (y1192 + 1634*v);
      int g = (y1192 - 833*v - 400*u);
      int b = (y1192 + 2066*u);

      if (r < 0) r = 0;
      else if (r > 262143) r = 262143;
      if (g < 0) g = 0;
      else if (g > 262143) g = 262143;
      if (b < 0) b = 0;
      else if (b > 262143) b = 262143;

      rgba[yp] = 0xff000000 | ((b << 6) & 0xff0000) | ((g >> 2) & 0xff00) | ((r >> 10) & 0xff);
    }
  }

  lua_pushlightuserdata(L, rgba);
  lua_pushinteger(L, 4*width*height);
  lua_pushstring(L, "byte");
  return 3;
}

static int lua_camera_getInfo(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  struct CameraInfo cameraInfo;
  if (cam->camera->getCameraInfo(cam->id, &cameraInfo) != 0) {
    lua_pushnil(L);
    return 1;
  }

  lua_createtable(L, 0, 2);
  lua_pushinteger(L, cameraInfo.facing);
  lua_setfield(L, -2, "facing");
  lua_pushinteger(L, cameraInfo.orientation);
  lua_setfield(L, -2, "orientation");
  return 1;
}

static int lua_camera_getImage(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  lua_pop(L, lua_gettop(L));

  if (!(cam->buf)) {
    lua_pushnil(L);
    return 1;
  }

  lua_pushlightuserdata(L, cam->buf);
  lua_pushinteger(L, cam->datalen);
  lua_pushstring(L, "byte");
  return 3;
  //  return lua_camera_yuv420torgba(L);
}

static int lua_camera_getFrameNumber(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  lua_pushinteger(L, cam->nframe);
  return 1;
}

static const struct luaL_reg camera_functions[] = {
  {"new", lua_camera_new},
  {"getNumberOfCameras", lua_camera_getNumberOfCameras},
  {"yuv420torgba", lua_camera_yuv420torgba},
  {NULL, NULL}
};

static const struct luaL_reg camera_methods[] = {
  {"connect", lua_camera_connect},
  {"disconnect", lua_camera_disconnect},

  {"getParameters", lua_camera_getParameters},
  {"setParameters", lua_camera_setParameters},

  {"setNotifyCallback", lua_camera_setNotifyCallback},
  {"setDataCallback", lua_camera_setDataCallback},
  {"setListener", lua_camera_setListener},
  {"setPreviewCallbackFlags", lua_camera_setPreviewCallbackFlags},
  {"setPreviewDisplay", lua_camera_setPreviewDisplay},
  {"setPreviewTexture", lua_camera_setPreviewTexture},

  {"startPreview", lua_camera_startPreview},
  {"stopPreview", lua_camera_stopPreview},
  {"startRecording", lua_camera_startRecording},
  {"stopRecording", lua_camera_stopRecording},

  {"getFrameNumber", lua_camera_getFrameNumber},
  {"getImage", lua_camera_getImage},
  {"getInfo", lua_camera_getInfo},
  {"__gc", lua_camera_delete},
  {"__tostring", lua_camera_tostring},
  {NULL, NULL}
};


#ifdef __cplusplus
extern "C"
#endif
int luaopen_nativecamera (lua_State *L) {
  LOGI("luaopen_nativecamera called");

  luaL_newmetatable(L, MT_NAME);
  // OO access: mt.__index = mt
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_register(L, NULL, camera_methods);

  luaL_register(L, "nativecamera", camera_functions);
  return 1;
}

// Allow module to be loaded with require("android.nativecamera")
#ifdef __cplusplus
extern "C"
#endif
int luaopen_android_nativecamera (lua_State *L) {
  return luaopen_nativecamera(L);
}
