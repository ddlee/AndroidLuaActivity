LOCAL_PATH := $(call my-dir)

# sensor module to access Android sensor (accel, mag, gyr, etc.)
include $(CLEAR_VARS)
LOCAL_MODULE := sensor
LOCAL_LDLIBS := -landroid -llog
LOCAL_SRC_FILES := luasensor.cpp
LOCAL_SHARED_LIBRARIES := lua-activity
include $(BUILD_SHARED_LIBRARY)
