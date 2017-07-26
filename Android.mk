LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
#LOCAL_C_INCLUDES:= $(LOCAL_PATH)/librabbitmq
LOCAL_C_INCLUDES:= external/rabbitmq-c/include
LOCAL_SRC_FILES:= librabbitmq/amqp_api.c \
		  librabbitmq/amqp_consumer.c \
		  librabbitmq/amqp_hostcheck.c \
		  librabbitmq/amqp_table.c \
		  librabbitmq/amqp_time.c \
		  librabbitmq/amqp_connection.c \
		  librabbitmq/amqp_framing.c \
		  librabbitmq/amqp_mem.c \
		  librabbitmq/amqp_socket.c \
		  librabbitmq/amqp_tcp_socket.c \
		  librabbitmq/amqp_url.c

LOCAL_MODULE := librabbitmq
#LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)


ifeq ($(HOST_OS), linux)
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= external/rabbitmq-c/include
LOCAL_SRC_FILES:= examples/amqp_bind.c \
		  examples/utils.c
LOCAL_MODULE := amqp_bind
LOCAL_SHARED_LIBRARIES:= librabbitmq
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
endif
