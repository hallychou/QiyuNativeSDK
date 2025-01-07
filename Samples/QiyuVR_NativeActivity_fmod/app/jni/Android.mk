# /*******************************************************
# Copyright (c) 2021 IQIYISMART, Inc. All Rights Reserved.
# *******************************************************/

LOCAL_PATH := $(call my-dir)

QiyuNativeSDK_Include_ := 	$(LOCAL_PATH)/../../../../QiyuNativeSDK/Include
QiyuNativeSDK_Lib_ := 		$(LOCAL_PATH)/../.QiyuNativeSDK/$(APP_OPTIM)/jni/$(TARGET_ARCH_ABI)
3rdParty_Include_ := 		$(LOCAL_PATH)/../../../3rdParty/Include
3rdParty_Lib_ := 			$(LOCAL_PATH)/../../../3rdParty/Lib/$(APP_OPTIM)/jni/$(TARGET_ARCH_ABI)


include $(CLEAR_VARS)
LOCAL_MODULE 	:= libqiyuapi
LOCAL_SRC_FILES := $(QiyuNativeSDK_Lib_)/libqiyuapi.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE 	:= libassimp
LOCAL_SRC_FILES := $(3rdParty_Lib_)/libassimp.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE 	:= libopencv_java3
LOCAL_SRC_FILES := $(3rdParty_Lib_)/libopencv_java3.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE 			:= fmod
LOCAL_SRC_FILES         := $(FMOD_API_ROOT)/core/lib/$(TARGET_ARCH_ABI)/libfmod$(FMOD_LIB_SUFFIX).so
LOCAL_EXPORT_C_INCLUDES := $(FMOD_API_ROOT)/core/inc
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE    := qiyuvr_nativeactivity_fmod
LOCAL_ARM_MODE	:= arm

LOCAL_C_INCLUDES := $(QiyuNativeSDK_Include_) \
					$(3rdParty_Include_) \
					$(3rdParty_Include_)/opencv_3_0_0 \
					$(3rdParty_Include_)/glm-0.9.7.0
LOCAL_SRC_FILES += QiyuVR_NativeActivity_fmod.cpp \
					QYRenderTarget.cpp \
					QYUtil.cpp \
					QYShader.cpp \
					QYMesh.cpp \
					QYModel.cpp \
					QYFmod.cpp

LOCAL_LDLIBS := -lEGL -lGLESv3 -landroid -llog

LOCAL_LDFLAGS := -u ANativeActivity_onCreate
LOCAL_STATIC_LIBRARIES := android_native_app_glue

LOCAL_SHARED_LIBRARIES := libqiyuapi \
						libassimp \
						libopencv_java3 \
						fmod
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
