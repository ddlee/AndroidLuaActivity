LOCAL_PATH := $(call my-dir)

# native camera module to access Android camera
include $(CLEAR_VARS)
LOCAL_MODULE := nativecamera
LOCAL_SRC_FILES := luanativecamera.cpp
LOCAL_C_INCLUDES += $(JNI_PATH)/android/include
LOCAL_LDFLAGS += -L$(JNI_PATH)/android/lib/$(TARGET_ARCH_ABI)
LOCAL_LDLIBS := -lcamera_client -lutils -lbinder -llog
LOCAL_SHARED_LIBRARIES := lua-activity
include $(BUILD_SHARED_LIBRARY)

# JNI camera module to access Android camera
include $(CLEAR_VARS)
LOCAL_MODULE := jnicamera
LOCAL_SRC_FILES := luajnicamera.cpp
LOCAL_C_INCLUDES += $(JNI_PATH)/android/include
LOCAL_LDFLAGS += -L$(JNI_PATH)/android/lib/$(TARGET_ARCH_ABI)
LOCAL_LDLIBS := -lcamera_client -lutils -lbinder -llog
LOCAL_SHARED_LIBRARIES := lua-activity
include $(BUILD_SHARED_LIBRARY)
