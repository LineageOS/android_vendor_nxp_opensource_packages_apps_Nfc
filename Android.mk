ifeq ($(strip $(TARGET_USES_NQ_NFC)),true)
LOCAL_PATH:= $(call my-dir)

########################################
# NCI Configuration
########################################
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
        $(call all-java-files-under, src)

LOCAL_SRC_FILES += \
        $(call all-java-files-under, nci)

LOCAL_PACKAGE_NAME := NQNfcNci
LOCAL_CERTIFICATE := platform

LOCAL_JNI_SHARED_LIBRARIES := libnqnfc_nci_jni
LOCAL_JAVA_LIBRARIES := com.nxp.nfc.nq
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_STATIC_JAVA_LIBRARIES := android-support-core-utils

LOCAL_MODULE_OWNER := nxp
LOCAL_PRIVATE_PLATFORM_APIS := true
include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
endif
