/*******************************************************
Copyright (c) 2021 IQIYISMART, Inc. All Rights Reserved.
*******************************************************/
#ifndef _QYUTIL_H_
#define _QYUTIL_H_

#include <android/log.h>


#define OUTPUT_VERBOSE_ 0
static bool enable_OUTPUT_VERBOSE_ = false;
#define NAME_ "QiyuVR_NativeActivity"
#define LOGI_(...) __android_log_print(ANDROID_LOG_INFO, NAME_, __VA_ARGS__)
#define LOGW_(...) __android_log_print(ANDROID_LOG_WARN, NAME_, __VA_ARGS__)
#define LOGE_(...) __android_log_print(ANDROID_LOG_ERROR, NAME_, __VA_ARGS__)
#if OUTPUT_VERBOSE_
#define LOGV_(...) if (enable_OUTPUT_VERBOSE_) __android_log_print(ANDROID_LOG_VERBOSE, NAME_, __VA_ARGS__)
#else
#define LOGV_(...)
#endif


namespace QY
{
	void CheckGlError(const char* file, int line);
	void CheckEglError(const char* file, int line);
};

#define CHECK_GL_ 1
#if CHECK_GL_
#define QY_GL(func) func; QY::CheckGlError(__FILE__, __LINE__);
#define QY_EGL(func) func; QY::CheckEglError(__FILE__, __LINE__);
#else
#define QY_GL(func) func;
#define QY_EGL(func) func;
#endif


#endif//_QYUTIL_H_