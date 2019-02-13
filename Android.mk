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
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform

LOCAL_JNI_SHARED_LIBRARIES := libsn100nfc_nci_jni
LOCAL_JAVA_LIBRARIES := com.nxp.nfc.nq
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_STATIC_JAVA_LIBRARIES := android-support-core-utils
LOCAL_STATIC_JAVA_LIBRARIES += android.hardware.secure_element-V1.0-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.nxp.hardware.nfc-V1.1-java

LOCAL_MODULE_OWNER := nxp
include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
