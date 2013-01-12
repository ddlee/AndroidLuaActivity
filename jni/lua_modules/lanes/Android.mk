LOCAL_PATH := $(call my-dir)

# lua lanes core module
include $(CLEAR_VARS)
LOCAL_MODULE := lua51-lanes
LOCAL_SRC_FILES := \
	lanes.c \
	threading.c \
	tools.c \
	keeper.c \
	pthread_hack.c
LOCAL_SHARED_LIBRARIES := lua-activity
include $(BUILD_SHARED_LIBRARY)
