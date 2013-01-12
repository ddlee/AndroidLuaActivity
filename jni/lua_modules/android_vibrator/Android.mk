LOCAL_PATH := $(call my-dir)

# vibrator module to access Android vibrator
include $(CLEAR_VARS)
LOCAL_MODULE := vibrator
LOCAL_SRC_FILES := luavibrator.cpp
LOCAL_LDLIBS := -landroid -llog
LOCAL_SHARED_LIBRARIES := lua-activity
include $(BUILD_SHARED_LIBRARY)
