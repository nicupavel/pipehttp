LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ph

LOCAL_SRC_FILES := \
    ph.c \
    server.c \
    config.c \
    dlist.c \
    http.c \
    messages.c

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/

include $(BUILD_EXECUTABLE)

