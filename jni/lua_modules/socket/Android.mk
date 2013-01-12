LOCAL_PATH := $(call my-dir)

# Need to build as libsocket_core.so
# since Android libs directory cannot have subdirectories
# and because all-in-one loader loads before Android asset loader
include $(CLEAR_VARS)
LOCAL_MODULE := socket_core
LOCAL_SRC_FILES := \
	luasocket.c \
	timeout.c \
	buffer.c \
	io.c \
	auxiliar.c \
	options.c \
	inet.c \
	tcp.c \
	udp.c \
	except.c \
	select.c \
	usocket.c
LOCAL_SHARED_LIBRARIES := lua-activity
include $(BUILD_SHARED_LIBRARY)

# lua socket mime.core module (all-in-one loader should open libmime.so)
include $(CLEAR_VARS)
LOCAL_MODULE := mime_core
LOCAL_SRC_FILES := mime.c
LOCAL_SHARED_LIBRARIES := lua-activity
include $(BUILD_SHARED_LIBRARY)
