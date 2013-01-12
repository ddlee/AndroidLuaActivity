JNI_PATH := $(call my-dir)
#NDK_PATH := /Users/ddlee/Android/android-ndk-r8d

# luacore, freetype2, allegro5:
include $(call all-subdir-makefiles)

LOCAL_PATH := $(JNI_PATH)

# Native main activity library
# Note: Android runtime loader is very limited
# Native activity library cannot dlopen any custom shared libraries
# Need to statically link lua core module here
include $(CLEAR_VARS)
LOCAL_MODULE    := lua-activity

LOCAL_SRC_FILES := src/activity.cpp
# Statically compile in jnicontext:
LOCAL_SRC_FILES += src/jnicontext.cpp

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_LDLIBS    := -llog -landroid -ldl
LOCAL_STATIC_LIBRARIES := luacore-static
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include \
	 $(LOCAL_PATH)/lua-5.1.4/include
include $(BUILD_SHARED_LIBRARY)

# Custom lua modules are linked to liblua-activity.so
