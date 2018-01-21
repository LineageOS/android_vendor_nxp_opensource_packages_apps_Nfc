VOB_COMPONENTS := vendor/nxp/opensource/external/libnfc-nci/src
NFA := $(VOB_COMPONENTS)/nfa
NFC := $(VOB_COMPONENTS)/nfc

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

LOCAL_PRELINK_MODULE := false

ifneq ($(NCI_VERSION),)
LOCAL_CFLAGS += -DNCI_VERSION=$(NCI_VERSION) -O0 -g
endif

LOCAL_CFLAGS += -Wall -Wextra -Wno-unused-parameter

#NXP PN547 Enable
LOCAL_CFLAGS += -DNXP_EXTNS=TRUE
LOCAL_CFLAGS += -DNFC_NXP_NON_STD_CARD=FALSE
LOCAL_CFLAGS += -DNFC_NXP_HFO_SETTINGS=FALSE

#Enable HCE-F specific
LOCAL_CFLAGS += -DNXP_NFCC_HCE_F=TRUE

NFC_POWER_MANAGEMENT:= TRUE
ifeq ($(NFC_POWER_MANAGEMENT),TRUE)
LOCAL_CFLAGS += -DNFC_POWER_MANAGEMENT=TRUE
else
LOCAL_CFLAGS += -DNFC_POWER_MANAGEMENT=FALSE
endif

LOCAL_CFLAGS += -DNXP_LDR_SVC_VER_2=TRUE

LOCAL_SRC_FILES := $(call all-subdir-cpp-files) $(call all-subdir-c-files)

LOCAL_C_INCLUDES += \
    frameworks/native/include \
    libnativehelper/include/nativehelper \
    $(NFA)/include \
    $(NFA)/brcm \
    $(NFC)/include \
    $(NFC)/brcm \
    $(NFC)/int \
    $(VOB_COMPONENTS)/hal/include \
    $(VOB_COMPONENTS)/hal/int \
    $(VOB_COMPONENTS)/include \
    $(VOB_COMPONENTS)/gki/ulinux \
    $(VOB_COMPONENTS)/gki/common

ifeq ($(NFC_NXP_ESE),TRUE)
LOCAL_C_INCLUDES += vendor/nxp/opensource/external/libnfc-nci/p61-jcop-kit/include

endif

LOCAL_SHARED_LIBRARIES := \
    libicuuc \
    libnativehelper \
    libcutils \
    libutils \
    liblog \
    libnqnfc-nci

ifeq ($(NFC_NXP_ESE),TRUE)
LOCAL_SHARED_LIBRARIES += libnqp61-jcop-kit
endif

#LOCAL_STATIC_LIBRARIES := libxml2
ifeq (true,$(TARGET_IS_64_BIT))
LOCAL_MULTILIB := 64
LOCAL_CFLAGS += -DNFC_ARCH_TYPE=64
else
LOCAL_MULTILIB := 32
LOCAL_CFLAGS += -DNFC_ARCH_TYPE=32
endif

LOCAL_MODULE := libnqnfc_nci_jni
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := nxp

include $(BUILD_SHARED_LIBRARY)
