LOCAL_PATH := $(call my-dir)

# toast module to show Android toast messages
include $(CLEAR_VARS)
LOCAL_MODULE := toast
LOCAL_SRC_FILES := luatoast.cpp
LOCAL_LDLIBS := -landroid -llog
LOCAL_SHARED_LIBRARIES := lua-activity
include $(BUILD_SHARED_LIBRARY)
