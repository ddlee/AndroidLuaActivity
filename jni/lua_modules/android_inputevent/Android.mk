LOCAL_PATH := $(call my-dir)

# inputevent modules to query Android Input Events
include $(CLEAR_VARS)
LOCAL_MODULE := inputevent
LOCAL_SRC_FILES := luainputevent.cpp
LOCAL_LDLIBS := -landroid -llog
LOCAL_SHARED_LIBRARIES := lua-activity
include $(BUILD_SHARED_LIBRARY)
