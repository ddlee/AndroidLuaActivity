#ifndef jnicontext_h
#define jnicontext_h

#include <stdlib.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/native_activity.h>

#ifdef __cplusplus
extern "C" {
#endif

void jniSetJavaVM(JavaVM* vm);
JavaVM* jniGetJavaVM();
JNIEnv* jniGetEnv();

int jniSetContext(jobject context);
jobject jniGetActivity();
jobject jniGetApplication();
jobject jniGetApplicationContext();
  
int jniSetAssetManager(AAssetManager *a);
AAssetManager *jniGetAssetManager();
int jniSetAssetManager(AAssetManager *a);

jclass jniFindClass(const char *name);
const char* jniGetFilesDir(ANativeActivity *activity);

#ifdef __cplusplus
}
#endif

#endif
