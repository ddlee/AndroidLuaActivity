/*
  Utility functions to define Android context
  (activity and application jobjects)
  for use by JNI in other modules
*/

#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "jnicontext.h"

#ifndef LOG_TAG
#define LOG_TAG "lua"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static JavaVM* gJavaVM;
static jobject gActivity;
static jobject gApplication;
static jobject gApplicationContext;
static AAssetManager* gAssetManager;

// Called to save JavaVM if library is loaded from Java:
extern "C"
int JNI_OnLoad(JavaVM* vm, void* reserved) {
  LOGI("JNI_OnLoad: saving gJavaVM");
  jniSetJavaVM(vm);

  return JNI_VERSION_1_4;
}

extern "C"
void jniSetJavaVM(JavaVM* vm) {
  LOGI("jniSetJavaVM");
  gJavaVM = vm;
}

extern "C"
JavaVM* jniGetJavaVM() {
  return gJavaVM;
}

// Utility function to get JNIEnv
extern "C"
JNIEnv* jniGetEnv() {
  JNIEnv *env;
  if (gJavaVM == 0) {
    LOGE("Invalid global Java VM");
    return 0;
  }

  int status;
  status = gJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
  if (status < 0) {
    LOGW("Failed to get JNI environment, trying to attach thread");
    // Try to attach native thread to JVM:
    status = gJavaVM->AttachCurrentThread(&env, 0);
    if (status < 0) {
      LOGE("Failed to attach current thread to JVM");
      return 0;
    }
  }

  return env;
}

extern "C"
int jniSetContext(jobject context) {
  JNIEnv *env = jniGetEnv();
  if (env == 0) {
    LOGE("Could not get JNIEnv");
    return -1;
  }

  gActivity = context;
  LOGI("Saving Android activity object %p", gActivity);

  // Application object
  jmethodID methodGetApplication = 
    env->GetMethodID(env->GetObjectClass(gActivity),
		     "getApplication",
		     "()Landroid/app/Application;");
  if (methodGetApplication == 0) {
    LOGE("Failed to get getApplication method");
    return -1;
  }
  gApplication = env->CallObjectMethod(gActivity,
				       methodGetApplication);
  LOGI("Saving Android application object %p", gApplication);

  // Application context
  jclass contextClass = env->FindClass("android/content/Context");
  if (contextClass == 0) {
    LOGE("Could not get context object class");
    return -1;
  }
  jmethodID getApplicationContextMethod =
    env->GetMethodID(contextClass,
		     "getApplicationContext",
		     "()Landroid/content/Context;");
  if (getApplicationContextMethod == 0) {
    LOGE("Could not get getApplicationContext method");
    return -1;
  }
  gApplicationContext =
    env->CallObjectMethod(context,
			  getApplicationContextMethod);
  LOGI("Saving Android application context object %p", gApplicationContext);

  // If needed, set asset manager:
  if (!gAssetManager) {
    jniSetAssetManager(NULL);
  }

  return 0;
}

extern "C"
jobject jniGetActivity() {
  return gActivity;
}

extern "C"
jobject jniGetApplication() {
  return gApplication;
}

extern "C"
jobject jniGetApplicationContext() {
  return gApplicationContext;
}

extern "C"
int jniSetAssetManager(AAssetManager *a) {
  if (a) {
    gAssetManager = a;
    LOGI("Setting Android asset manager: %p", gAssetManager);
    return 0;
  }
  else {
    // Uses Java VM and application context to obtain Java asset manager
    // and then uses NDK AAssetManager_fromJava()
    // Note: In Android emulator, this seems to only work in main thread!
    JNIEnv *env = jniGetEnv();
    jobject context = jniGetApplicationContext();
    jmethodID methodGetAssets =
      env->GetMethodID(env->GetObjectClass(context),
		       "getAssets",
		       "()Landroid/content/res/AssetManager;");
    if (methodGetAssets == 0) {
      LOGE("Could not get getAssets method");
      return 0;
    }
    jobject localAssetManager = 
      env->CallObjectMethod(context,
			    methodGetAssets);
    if (localAssetManager == 0) {
      LOGE("Could not get local Java Asset Manager");
      return 0;
    }
    jobject globalAssetManager = env->NewGlobalRef(localAssetManager);
    if (globalAssetManager == 0) {
      LOGE("Could not get global Java Asset Manager");
      return 0;
    }

    gAssetManager = AAssetManager_fromJava(env, globalAssetManager);
    LOGI("Setting Android asset manager: %p", gAssetManager);
  }
}

extern "C"
AAssetManager *jniGetAssetManager() {
  return gAssetManager;
}

extern "C"
jclass jniFindClass(const char *name) {
  JNIEnv *env = jniGetEnv();

  // Default class loader will not work on custom Java files
  // getClassLoader method on native activity object
  jclass activityClass =
    env->FindClass("android/app/NativeActivity");
  jmethodID getClassLoaderMethod =
    env->GetMethodID(activityClass,
		      "getClassLoader",
		      "()Ljava/lang/ClassLoader;");
  LOGI("getClassLoaderMethod %p", getClassLoaderMethod);

  // Use Native Activity clazz
  jobject classLoaderObject =
    env->CallObjectMethod(gActivity, getClassLoaderMethod);
  LOGI("classLoaderObject %p", classLoaderObject);

  jclass javaClassLoader = env->FindClass("java/lang/ClassLoader");
  jmethodID loadClassMethod =
    env->GetMethodID(javaClassLoader,
		     "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
  LOGI("loadClassMethod %p", loadClassMethod);

  jclass classFind =
    (jclass)env->CallObjectMethod(classLoaderObject,
				  loadClassMethod,
				  env->NewStringUTF(name));
  LOGI("classFind %p", classFind);

  return classFind;
}

// Hack since activity->internalDataPath is broken in Android 2.3
extern "C"
const char* jniGetFilesDir(ANativeActivity *activity) {
  JNIEnv *env = jniGetEnv();
  jclass activityClass = env->FindClass("android/app/NativeActivity");
  jmethodID getFilesDirMethod = env->GetMethodID(activityClass,
						 "getFilesDir",
						 "()Ljava/io/File;");
  jobject filesDirObj = env->CallObjectMethod(activity->clazz,
					      getFilesDirMethod);
  jclass fileClass = env->FindClass("java/io/File");
  jmethodID getPathMethod = env->GetMethodID(fileClass,
					     "getPath",
					     "()Ljava/lang/String;");
  jstring pathObj = (jstring) env->CallObjectMethod(filesDirObj,
						    getPathMethod);
  const char* filesDir = env->GetStringUTFChars(pathObj, NULL);

  // TODO: StringUTFChars needs to be released after it's used...
  //  env->ReleaseStringUTFChars(pathObj, dir);
  return filesDir;
}

/*
JNIEXPORT jint JNICALL
Java_com_example_lua_Main_jniSetContext(JNIEnv *env, jobject thiz,
					jobject context) {

  return jniSetContext(context);
}
*/
