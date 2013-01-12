LOCAL_PATH := $(call my-dir)

# OpenGL EGL module
include $(CLEAR_VARS)
LOCAL_MODULE := egl
LOCAL_SRC_FILES := luaegl.cpp
LOCAL_LDLIBS := -landroid -llog -lEGL
LOCAL_SHARED_LIBRARIES := lua-activity
include $(BUILD_SHARED_LIBRARY)
