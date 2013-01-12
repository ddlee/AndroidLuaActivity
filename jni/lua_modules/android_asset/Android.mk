LOCAL_PATH := $(call my-dir)

# asset module to access Android package assets
include $(CLEAR_VARS)
LOCAL_MODULE := asset
LOCAL_LDLIBS := -landroid -llog
LOCAL_SRC_FILES := luaasset.cpp
LOCAL_SHARED_LIBRARIES := lua-activity
include $(BUILD_SHARED_LIBRARY)
