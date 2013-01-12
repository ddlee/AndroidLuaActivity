LOCAL_PATH := $(call my-dir)

# TTS module for Android text to speech
include $(CLEAR_VARS)
LOCAL_MODULE := tts
LOCAL_SRC_FILES := luatts.cpp
LOCAL_LDLIBS := -landroid -llog
LOCAL_SHARED_LIBRARIES := lua-activity
include $(BUILD_SHARED_LIBRARY)
