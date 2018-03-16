VOB_COMPONENTS := vendor/nxp/opensource/external/libnfc-nci/src
LIBNFC_PATH := vendor/nxp/opensource/external/libnfc-nci
NFA := $(VOB_COMPONENTS)/nfa
NFC := $(VOB_COMPONENTS)/nfc

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

LOCAL_PRELINK_MODULE := false

ifneq ($(NCI_VERSION),)
LOCAL_CFLAGS += -DNCI_VERSION=$(NCI_VERSION) -O0 -g
endif

NFC_NXP_ESE:= TRUE

LOCAL_CFLAGS += -Wall -Wextra -Wno-unused-parameter -Werror
LOCAL_CFLAGS += -DNXP_EXTNS=TRUE
LOCAL_SRC_FILES := $(call all-subdir-cpp-files) $(call all-subdir-c-files)

LOCAL_C_INCLUDES += \
    external/libxml2/include \
    frameworks/native/include \
    $(NFA)/include \
    $(NFA)/brcm \
    $(NFC)/include \
    $(NFC)/brcm \
    $(NFC)/int \
    $(VOB_COMPONENTS)/hal/include \
    $(VOB_COMPONENTS)/hal/int \
    $(VOB_COMPONENTS)/include \
    $(VOB_COMPONENTS)/gki/ulinux \
    $(VOB_COMPONENTS)/gki/common \
    $(LIBNFC_PATH)/utils/include \
    $(LIBNFC_PATH)/extns/impl

ifeq ($(NFC_NXP_ESE),TRUE)
LOCAL_C_INCLUDES += vendor/nxp/opensource/external/libnfc-nci/p61-jcop-kit/include

endif

LOCAL_SHARED_LIBRARIES := \
    libicuuc \
    libnativehelper \
    libcutils \
    libutils \
    liblog \
    libnqnfc-nci \
    libchrome \
    libbase \

ifeq ($(NFC_NXP_ESE),TRUE)
LOCAL_SHARED_LIBRARIES += libnqp61-jcop-kit
endif

LOCAL_STATIC_LIBRARIES := libxml2
ifeq (true,$(TARGET_IS_64_BIT))
LOCAL_MULTILIB := 64
else
LOCAL_MULTILIB := 32
endif

LOCAL_MODULE := libnqnfc_nci_jni
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := nxp

include $(BUILD_SHARED_LIBRARY)
