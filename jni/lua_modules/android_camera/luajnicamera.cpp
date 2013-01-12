/*
  Lua module to interface to Android camera
  Request shim.CameraClass
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <android/log.h>

#include <jni.h>
#include "jnicontext.h"

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

#define MT_NAME "jnicamera"

#ifndef LOG_TAG
#define LOG_TAG "lua"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

// Java class of CameraJNI stored at luaopen_():
static jclass cameraJNIClass;

class CameraClass {
public:
  JNIEnv *env;
  jobject camera;
  int id;

  int nFrame;  // frame number counter
  jbyteArray dataByteArray; // Java byte array containing last frame
  jbyte *dataBytePtr; // Java byte pointer mapped from last frame
  long dataLen;

  // Static method to get total number of cameras:
  static int getNumberOfCameras() {
    JNIEnv *env = jniGetEnv();
    jmethodID methodID =
      env->GetStaticMethodID(cameraJNIClass, "getNumberOfCameras", "()I");
    LOGI("getNumberOfCamerasMethod: %p", methodID);
    return env->CallStaticIntMethod(cameraJNIClass, methodID);
  }
  
  // Constructor:
  CameraClass():id(0), nFrame(0),
		dataByteArray(0), dataBytePtr(0), dataLen(0) {
    env = jniGetEnv();

    // Get constructor method:
    jmethodID initMethod =
      env->GetMethodID(cameraJNIClass, "<init>", "()V");
    LOGI("initMethod: %p", initMethod);

    // Create local Java object:
    jobject cameraLocal = env->NewObject(cameraJNIClass, initMethod);
    if (!cameraLocal) LOGE("Could not construct CameraJNI Java object");

    // Global reference to prevent garbage collection:
    camera = env->NewGlobalRef(cameraLocal);
    LOGI("camera: %p", camera);
  }

  virtual ~CameraClass() {
    LOGI("CameraClass destructor");

    if (dataByteArray && dataBytePtr) {
      env->ReleaseByteArrayElements(dataByteArray, dataBytePtr, JNI_ABORT);
    }
    env->DeleteGlobalRef(camera);
  }

  void open(int n) {
    id = n;
    jmethodID openMethod =
      env->GetMethodID(cameraJNIClass, "open", "(I)V");
    env->CallVoidMethod(camera, openMethod, id);
  }
  void release() {
    jmethodID releaseMethod =
      env->GetMethodID(cameraJNIClass, "release", "()V");
    env->CallVoidMethod(camera, releaseMethod);
  }

  void startPreview() {
    jmethodID setContextMethod =
      env->GetMethodID(cameraJNIClass, "setContext", "(J)V");
    env->CallVoidMethod(camera, setContextMethod, (long)this);

    jmethodID startPreviewMethod =
      env->GetMethodID(cameraJNIClass, "startPreview", "()V");
    env->CallVoidMethod(camera, startPreviewMethod);
  }
  void stopPreview() {
    jmethodID stopPreviewMethod =
      env->GetMethodID(cameraJNIClass, "stopPreview", "()V");
    env->CallVoidMethod(camera, stopPreviewMethod);
  }
  void setPreviewSize(int width, int height) {
    jmethodID setPreviewSizeMethod =
      env->GetMethodID(cameraJNIClass, "setPreviewSize", "(II)V");  
    env->CallVoidMethod(camera, setPreviewSizeMethod, width, height);
  }
  void setPreviewTexture(int texName) {
    jmethodID setPreviewTextureMethod =
      env->GetMethodID(cameraJNIClass, "setPreviewTexture", "(I)V");
    env->CallVoidMethod(camera, setPreviewTextureMethod, texName);
  }

  void setData(JNIEnv *env, const jbyteArray data) {
    dataLen = env->GetArrayLength(data);

    if (nFrame % 100 == 0)
      LOGI("CameraClass.setData[%d]: %d bytes", nFrame, dataLen);

    if (dataByteArray && dataBytePtr) {
      // Release old data without copying:
      env->ReleaseByteArrayElements(dataByteArray, dataBytePtr, JNI_ABORT);
    }
    dataByteArray = data;
    dataBytePtr = env->GetByteArrayElements(dataByteArray, NULL);
  }

  const char* getParameters() {
    // TODO: Doesn't work, truncated characters!
    jmethodID getParametersMethod =
      env->GetMethodID(cameraJNIClass, "getParameters", "()Ljava/lang/String;");
    jstring pString = (jstring) env->CallObjectMethod(camera, getParametersMethod);
    const char* s = env->GetStringUTFChars(pString, NULL);
    // Assume that the VM doesn't change this memory before Lua returns:
    env->ReleaseStringUTFChars(pString, s);
    return s;
  }

  int callIntMethod(const char *name) {
    jmethodID intMethod =
      env->GetMethodID(cameraJNIClass, name, "()I");
    if (intMethod == 0) {
      return -1;
    }
    return (int)env->CallIntMethod(camera, intMethod);
  }
};

// This is called from Java in onPreviewFrame() in CameraJNI:
extern "C" JNIEXPORT void JNICALL
Java_shim_CameraJNI_nativePreviewFrameCallback(JNIEnv *env, jclass cls,
					       jbyteArray data, jlong ptr) {
  //  LOGI("nativePreviewFrameCallback: %d", env->GetArrayLength(data));
  // TODO: Check to make sure accessing C++ class members works robustly:
  CameraClass *p = (CameraClass *)ptr;
  p->nFrame++;
  p->setData(env, data);
  return;
}


static CameraClass* lua_checkcameraclass(lua_State *L, int narg) {
  void *ud = luaL_checkudata(L, narg, MT_NAME);
  luaL_argcheck(L, *(CameraClass **)ud != NULL, narg, "invalid object");
  return *(CameraClass **)ud;
}

static int lua_camera_getNumberOfCameras(lua_State *L) {
  int n = CameraClass::getNumberOfCameras();
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

  cam->open(id);
  return 0;
}

static int lua_camera_disconnect(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);

  cam->release();
  return 0;
}
  
static int lua_camera_getParameters(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);

  const char* pstr = cam->getParameters();
  lua_pushstring(L, pstr);
  return 1;
}

static int lua_camera_setParameters(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);

  int narg = lua_gettop(L);
  for (int i = 2; i < narg; i+=2) {
    const char *key = luaL_checkstring(L, i);
    const char *value = luaL_checkstring(L, i+1);
  }

  return 0;
}

static int lua_camera_startPreview(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  LOGI("startPreview");

  cam->startPreview();
  return 0;
}
static int lua_camera_stopPreview(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);

  cam->stopPreview();
  return 0;
}
static int lua_camera_setPreviewTexture(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  unsigned int texName = luaL_checkint(L, 2);
  cam->setPreviewTexture(texName);
  return 0;
}
static int lua_camera_setPreviewSize(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  int width = luaL_checkint(L, 2);
  int height = luaL_checkint(L, 3);
  cam->setPreviewSize(width, height);
  return 0;
}
static int lua_camera_getPreviewSize(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  int width = cam->callIntMethod("getPreviewSizeWidth");
  int height = cam->callIntMethod("getPreviewSizeHeight");
  lua_pushinteger(L, width);
  lua_pushinteger(L, height);
  return 2;
}
static int lua_camera_getPreviewFormat(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  int format = cam->callIntMethod("getPreviewFormat");
  lua_pushinteger(L, format);
  return 1;
}

static int lua_camera_startRecording(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);

  return 0;
}

static int lua_camera_stopRecording(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);

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
		  cam, cam->id, cam->nFrame);
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

  return 0;
}

static int lua_camera_getImage(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  
  //  if (!dataBytePtr) return 0;

  lua_pushlightuserdata(L, (char *) cam->dataBytePtr);
  lua_pushinteger(L, cam->dataLen);
  lua_pushstring(L, "byte");
  return 3;
  //  return lua_camera_yuv420torgba(L);
}

static int lua_camera_getFrameNumber(lua_State *L) {
  CameraClass *cam = lua_checkcameraclass(L, 1);
  lua_pushinteger(L, cam->nFrame);
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

  {"startPreview", lua_camera_startPreview},
  {"stopPreview", lua_camera_stopPreview},
  {"setPreviewTexture", lua_camera_setPreviewTexture},
  {"setPreviewSize", lua_camera_setPreviewSize},
  {"getPreviewSize", lua_camera_getPreviewSize},
  {"getPreviewFormat", lua_camera_getPreviewFormat},

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
int luaopen_jnicamera (lua_State *L) {
  LOGI("luaopen_jnicamera called");

  JNIEnv *env = jniGetEnv();

  // clazz = env->FindClass("android/hardware/Camera");
  // Find encapsulated Java class in src/shim/CameraJNI.java
  cameraJNIClass = jniFindClass("shim/CameraJNI");
  LOGI("cameraJNIClass: %p", cameraJNIClass);
  if (!cameraJNIClass)
    luaL_error(L, "Cannot find class: CameraJNI");

  luaL_newmetatable(L, MT_NAME);
  // OO access: mt.__index = mt
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_register(L, NULL, camera_methods);

  luaL_register(L, "jnicamera", camera_functions);
  return 1;
}

// Allow module to be loaded with require("android.jnicamera")
#ifdef __cplusplus
extern "C"
#endif
int luaopen_android_jnicamera (lua_State *L) {
  return luaopen_jnicamera(L);
}
