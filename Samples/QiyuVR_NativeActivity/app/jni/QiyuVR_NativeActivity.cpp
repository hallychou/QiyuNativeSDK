/*******************************************************
Copyright (c) 2021 IQIYISMART, Inc. All Rights Reserved.
*******************************************************/

/**
 * NOTE! Simple sample. For reference only.
 ************************************************
 *ATTENTION! <TriggerR> <<Right Controller>> to Switch the TestGroup(refer to 'enum TestGroup').
 ************************
 *TestGroup*:<<< TG_Default >>>
 **<<Right Controller>>
 ***<GripR> 	- vibrate this controller(amplitude:0.5f, duration:200.0f);
				  test FMOD, playSound2.//DELETE!
 ***<Press_A> 	- qiyu_SetTrackingOriginMode, switch between GroundMode and DeviceMode.
				  test FMOD, playSound3.//DELETE!
 ***<Press_B> 	- switch between 4 qiyu_FoveationLevel(FL_None, FL_Low, FL_Medium, FL_High, FL_Custom).
 **<<Left Controller>>
 ***<TriggerL>	- vibrate this controller(amplitude:1.0f, duration:4.0f).
 				  test FMOD, playSound1.//DELETE!
 ***<GripL>		- stop vibrate this controller. (press to rotate the teapot; release will stop the teapots.);
 ***<Press_X> 	- test eye buffer size changed. switch between scale 0.7f and 0.2f.
 ***<Press_Y> 	- swith between MultiView and MultiPass. when MultiView test color is Red/Green for Left/Right eye.
 ************************
 *TestGroup*:<<< TG_QiyuPlatform >>>
 **<<Right Controller>>
 ***<Press_A> 	- test__qiyu_Platform_Init
 ***<Press_B> 	- test__qiyu_Platform_GetAccountInfo
 **<<Left Controller>>
 ***<GripL> 	- test__qiyu_Platform_GetDeepLink
 ***<Touch_X> 	- test__qiyu_Platform_IsAccountLogin
 ************************
 *TestGroup*:<<< TG_QiyuPrefs >>>
 **<<Right Controller>>
 ***<Press_A> 	- test__init_QiyuPrefs
 ***<Press_B> 	- test__set_QiyuPrefs
 ***<Touch_B> 	- test__get_QiyuPrefs
 **<<Left Controller>>
 ***<Press_X> 	- test__hasKey_QiyuPrefs
 ***<Press_Y> 	- test__delKey_QiyuPrefs
 ***<TriggerL> 	- test__delALL_QiyuPrefs
 ***<GripL> 	- test__save_QiyuPrefs
 ************************
 *TestGroup*:<<< TG_Boundary >>>
 **<<Right Controller>>
 ***<Press_B>	- qiyu_GetBoundaryGeometry.
 **<<Left Controller>>
 ***<TriggerL>	- qiyu_GetBoundaryDimensions.
 ***<Press_X> 	- qiyu_IsBoundaryVisible.
 ***<Press_Y> 	- qiyu_IsBoundaryBelowHeadVisible.
 ************************
 ************************************************
 */


#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h> // for prctl( PR_SET_NAME )
#include <android/log.h>
#include <android/native_window_jni.h> // for native window JNI
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <vector>
#include "QiyuApi.h"
#include "QYRenderTarget.h"
#include "QYShader.h"
#include "QYModel.h"
#include "QYUtil.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"
//#include "QYFmod.h"

#include "NDKHelper.h"
//-------------------------------------------------------------------------
// Preprocessor
//-------------------------------------------------------------------------
#define HELPER_CLASS_NAME \
  "com/sample/helper/NDKHelper"  // Class name of helper function


#if !defined(EGL_OPENGL_ES3_BIT_KHR)
#define EGL_OPENGL_ES3_BIT_KHR 0x0040
#endif

 /**
  * Foveation
  * https://www.khronos.org/registry/OpenGL/extensions/QCOM/QCOM_framebuffer_foveated.txt
  * https://www.khronos.org/registry/OpenGL/extensions/QCOM/QCOM_texture_foveated.txt
  * https://www.khronos.org/registry/OpenGL/extensions/QCOM/QCOM_texture_foveated2.txt
  * https://www.khronos.org/registry/OpenGL/extensions/QCOM/QCOM_texture_foveated_subsampled_layout.txt
  */
#ifndef GL_EXT_framebuffer_foveated

#define GL_FOVEATION_ENABLE_BIT_QCOM                    0x0001
#define GL_FOVEATION_SCALED_BIN_METHOD_BIT_QCOM         0x0002
#define GL_FOVEATION_SUBSAMPLED_LAYOUT_METHOD_BIT_QCOM  0x0004
//#define GL_TEXTURE_PREVIOUS_SOURCE_TEXTURE_QCOM         0x8BE8	//
//#define GL_TEXTURE_FOVEATED_FRAME_OFFSET_QCOM           0x8BE9	//
#define GL_TEXTURE_FOVEATED_FEATURE_BITS_QCOM           0x8BFB
#define GL_TEXTURE_FOVEATED_MIN_PIXEL_DENSITY_QCOM      0x8BFC
//#define GL_TEXTURE_FOVEATED_FEATURE_QUERY_QCOM          0x8BFD
//#define GL_TEXTURE_FOVEATED_NUM_FOCAL_POINTS_QUERY_QCOM 0x8BFE
//#define GL_FRAMEBUFFER_INCOMPLETE_FOVEATION_QCOM        0x8BFF
//#define GL_MAX_SHADER_SUBSAMPLED_IMAGE_UNITS_QCOM       0x8FA1

#ifdef GL_GLEXT_PROTOTYPES
//GL_APICALL void GL_APIENTRY glFramebufferFoveationConfigQCOM(GLuint fbo, GLuint numLayers, GLuint focalPointsPerLayer, GLuint requiredFeatures, GLuint* gotFeatures);
//GL_APICALL void GL_APIENTRY glFramebufferFoveationParametersQCOM(GLuint fbo, GLuint layer, GLuint focalPoint, GLfloat focalX, GLfloat focalY, GLfloat gainX, GLfloat gainY, GLfloat foveaArea);
GL_APICALL void GL_APIENTRY glTextureFoveationParametersQCOM(GLuint texure, GLuint layer, GLuint focalPoint, GLfloat focalX, GLfloat focalY, GLfloat gainX, GLfloat gainY, GLfloat foveaArea);
#endif

#define GL_APIENTRYP GL_APIENTRY*
typedef void (GL_APIENTRYP PFNGLTEXTUREFOVEATIONPARAMETERSEXT)(GLuint texture, GLuint layer, GLuint focalPoint, GLfloat focalX, GLfloat focalY, GLfloat gainX, GLfloat gainY, GLfloat foveaArea);
PFNGLTEXTUREFOVEATIONPARAMETERSEXT glTextureFoveationParametersQCOM = NULL;
//
//typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERFOVEATIONCONFIGEXT)(GLuint fbo, GLuint numLayers, GLuint focalPointsPerLayer, GLuint requiredFeatures, GLuint* gotFeatures);
//PFNGLFRAMEBUFFERFOVEATIONCONFIGEXT glFramebufferFoveationConfigQCOM = NULL;
////
//typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERFOVEATIONPARAMETERSEXT)(GLuint fbo, GLuint layer, GLuint focalPoint, GLfloat focalX, GLfloat focalY, GLfloat gainX, GLfloat gainY, GLfloat foveaArea);
//PFNGLFRAMEBUFFERFOVEATIONPARAMETERSEXT glFramebufferFoveationParametersQCOM = NULL;
//bool glTextureFoveationFrameOffsetQCOM = false;

#endif//GL_EXT_framebuffer_foveated

namespace QY_GL_EXT
{
	bool InitFunction_Foveation()
	{
		assert(glTextureFoveationParametersQCOM == NULL);
		glTextureFoveationParametersQCOM = (PFNGLTEXTUREFOVEATIONPARAMETERSEXT)eglGetProcAddress("glTextureFoveationParametersQCOM");
		if (glTextureFoveationParametersQCOM == NULL)
		{
			LOGE_("@@QY_GL_EXT::InitFunction_Foveation, glTextureFoveationParametersQCOM is not supported, fail to get proc address!");
			return false;
		}
		//assert(glFramebufferFoveationConfigQCOM == NULL);
		//glFramebufferFoveationConfigQCOM = (PFNGLFRAMEBUFFERFOVEATIONCONFIGEXT)eglGetProcAddress("glFramebufferFoveationConfigQCOM");
		//if (glFramebufferFoveationConfigQCOM == NULL)
		//{
		//	LOGE_("@@QY_GL_EXT::InitFunction_Foveation, glFramebufferFoveationConfigQCOM is not supported, fail to get proc address!");
		//	return false;
		//}
		//assert(glFramebufferFoveationParametersQCOM == NULL);
		//glFramebufferFoveationParametersQCOM = (PFNGLFRAMEBUFFERFOVEATIONPARAMETERSEXT)eglGetProcAddress("glFramebufferFoveationParametersQCOM");
		//if (glFramebufferFoveationParametersQCOM == NULL)
		//{
		//	LOGE_("@@QY_GL_EXT::InitFunction_Foveation, glFramebufferFoveationParametersQCOM is not supported, fail to get proc address!");
		//	return false;
		//}
		return true;
	}
	bool IsSupport_Foveation()
	{
		return glTextureFoveationParametersQCOM != NULL;
	}
}


enum TestGroup
{
	TG_Default = 0,
	TG_QiyuPlatform,
	TG_QiyuPrefs,
	TG_Boundary,

	TG_COUNT
};
static int s_TestGroup = TG_Default;
static const char* s_szTestGroup[TG_COUNT] = 
{
	"TG_Default",
	"TG_QiyuPlatform",
	"TG_QiyuPrefs",
	"TG_Boundary",
};


#define EYE_BUFFERS_COUNT 2//4	//FIXME! //TODO!

struct QYEyeBuffer//FIXME! //TODO! dynamic init and release
{
	QYRenderTarget qyRT[EYE_COUNT];			//<Press_X> - default eyeBuffer size: s_iScaledEyeTargetWidth_Height = deviceInfo.iEyeTargetWidth_Height * s_customScaleEyeTarget(0.7f);
	QYRenderTarget qyRTcustom[EYE_COUNT];	//<Press_X> - test eyeBuffer size: s_iScaledEyeTargetWidth_Height = deviceInfo.iEyeTargetWidth_Height * s_customScaleEyeTarget(0.2f);

	QYRenderTarget qyMultiViewRT;
	QYRenderTarget qyMultiViewRTcustom;
};
QYEyeBuffer qyEyeBuffers[EYE_BUFFERS_COUNT];

static const bool s_bIsProtectedContent = false;
static const int s_iMsaaSamples = 2;//4; //set s_iMsaaSamples > 1 to enable subsampled buffers.

static bool s_bIsCustomScaleRT = true;//true use s_fRTCustomScale; false use s_fRTScale;
static bool s_bIsMultiView = false;//true use MultiView; false use MultiPass;

static const float s_fRTScale		= 0.7f;//! recommended fixed scale for Qiyu devices.//test eye buffer size changed.
static const float s_fRTCustomScale	= 0.2f;//test eye buffer size changed.
static int s_iEyeTargetScaledW 	= 0;
static int s_iEyeTargetScaledH 	= 0;
static int s_iEyeTargetDeviceW 	= 0;
static int s_iEyeTargetDeviceH 	= 0;
static int s_idxEyeBuffer = 0;

struct QYEgl
{
	EGLint		eglMajorVersion;
	EGLint		eglMinorVersion;
	EGLDisplay	eglDisplay;
	EGLConfig	eglConfig;
	EGLSurface	eglSurface;
	EGLContext	eglContext;
};

struct QYApp
{
	QYEgl				qyEgl;
	char*				szExternalPath;
	ANativeActivity*	pANativeActivity;
};


class QYHandleRay
{
public:
	struct QYProgramAttribute
	{
		unsigned int    index;
		int             size;
		unsigned int    type;
		bool            normalized;
		int             stride;
		int				offset;
	};
	QYHandleRay(const QYProgramAttribute* pAttribs, int nAttribs, unsigned int* pIndices, int nIndices, const void* pVertexData, int bufferSize, int nVertices)
		: m_idVB(0)
		, m_idIB(0)
		, m_idVAO(0)
		, m_VertexCount(0)
		, m_IndexCount(0)
	{
		// VB
		QY_GL(glGenBuffers(1, &m_idVB));
		QY_GL(glBindBuffer(GL_ARRAY_BUFFER, m_idVB));
		QY_GL(glBufferData(GL_ARRAY_BUFFER, bufferSize, pVertexData, GL_STATIC_DRAW));
		QY_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
		// IB
		QY_GL(glGenBuffers(1, &m_idIB));
		QY_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idIB));
		QY_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndices * sizeof(unsigned int), pIndices, GL_STATIC_DRAW));
		QY_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
		// VAO
		QY_GL(glGenVertexArrays(1, &m_idVAO));
		QY_GL(glBindVertexArray(m_idVAO));
		QY_GL(glBindBuffer(GL_ARRAY_BUFFER, m_idVB));
		for (int i = 0; i < nAttribs; i++)
		{
			QY_GL(glEnableVertexAttribArray(pAttribs[i].index));
			QY_GL(glVertexAttribPointer(pAttribs[i].index, pAttribs[i].size, pAttribs[i].type, pAttribs[i].normalized, pAttribs[i].stride, (void*)(unsigned long long)(pAttribs[i].offset)));
		}
		QY_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
		QY_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idIB));
		QY_GL(glBindVertexArray(0));
		//
		m_VertexCount = nVertices;
		m_IndexCount = nIndices;
	}
	~QYHandleRay()
	{
		QY_GL(glDeleteVertexArrays(1, &m_idVAO));
		QY_GL(glDeleteBuffers(1, &m_idIB));
		QY_GL(glDeleteBuffers(1, &m_idVB));
	}
	void Draw()
	{
		QY_GL(glBindVertexArray(m_idVAO));
		QY_GL(glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, NULL));
		QY_GL(glBindVertexArray(0));
	}
private:
	unsigned int    m_idVB;
	unsigned int    m_idIB;
	unsigned int    m_idVAO;
	int             m_VertexCount;
	int             m_IndexCount;
};
QYHandleRay* g_pHandleRayLR = nullptr;
QYShader* g_pHandleRayShader = nullptr;
QYShader* g_pHandleRayShader_MultiView = nullptr;

QYModel* g_pHandleL = nullptr;
QYModel* g_pHandleR = nullptr;
QYShader* g_pHandleShader = nullptr;
QYShader* g_pHandleShader_MultiView = nullptr;

QYModel* g_pTeapot = nullptr;
QYShader* g_pTeapotShader = nullptr;
QYShader* g_pTeapotShader_MultiView = nullptr;

static const int s_iTeapotCount = 7;
static glm::vec3 s_teapotColor[s_iTeapotCount];
static glm::mat4 s_teapotModelMatrix[s_iTeapotCount];
static float s_fRotateAmount = 0.0f;
static bool s_bRotateTeapots = false;

qiyu_HeadPoseState g_headPoseState;
glm::mat4 g_matView[EYE_COUNT];
glm::mat4 g_matProj[EYE_COUNT];
static float g_fTrackingOffset = 0.0f;//FIXME!

static unsigned int g_uLastTime = 0;

static bool g_isPaused = true;
static bool g_isStartedVR = false;
static bool g_isInited = false;

struct ControllerState
{
	bool bControllerIsInit;
	qiyu_ControllerData qyCtrlData[CI_COUNT];
};

std::vector<qiyu_Vector3> g_pointsBoundaryGeometry;

static void QYApp_switchEyeTargetSize()
{
	s_bIsCustomScaleRT = !s_bIsCustomScaleRT;
	assert(s_iEyeTargetDeviceW != 0 && s_iEyeTargetDeviceH != 0);
	if (s_bIsCustomScaleRT)
	{
		s_iEyeTargetScaledW = s_iEyeTargetDeviceW * s_fRTCustomScale;
		s_iEyeTargetScaledH = s_iEyeTargetDeviceH * s_fRTCustomScale;
	}
	else
	{
		s_iEyeTargetScaledW = s_iEyeTargetDeviceW * s_fRTScale;
		s_iEyeTargetScaledH = s_iEyeTargetDeviceH * s_fRTScale;
	}
	LOGI_("$TEST$ test eye buffer ScaledWidth(%d), ScaledHeight(%d) (ScaledSize = deviceSize * Scale(%f) )", s_iEyeTargetScaledW, s_iEyeTargetScaledH, s_bIsCustomScaleRT ? s_fRTCustomScale : s_fRTScale);
	
	qiyu_PostSetEyeBufferSize(s_iEyeTargetScaledW, s_iEyeTargetScaledH);//!
}
static void QYApp_InitEyeTargetSize(int deviceW, int deviceH)
{
	s_iEyeTargetDeviceW = deviceW;
	s_iEyeTargetDeviceH = deviceH;	
	QYApp_switchEyeTargetSize();
}
static void QYApp_testMultiView()
{
	s_bIsMultiView = !s_bIsMultiView;
	LOGI_("$TEST$ test MultiView(%d))", s_bIsMultiView);
}
static bool s_bFoveationChanged = false;
static bool QYApp_testFoveation()
{
	qiyu_FoveationLevel foveatLevel_old;
	qiyu_FoveationParam foveationParam_old;
	if (!qiyu_GetFoveation(foveatLevel_old, foveationParam_old))
	{
		LOGE_("@@QYApp_testFoveation, fail to qiyu_GetFoveation(foveatLevel_old)");
		return false;
	}
	qiyu_FoveationLevel foveatLevel_new;
	foveatLevel_new = (qiyu_FoveationLevel)(((int)foveatLevel_old + 1) % FL_COUNT);
	if (foveatLevel_new == FL_Custom)
	{
		qiyu_FoveationParam customFoveationParam;
		customFoveationParam.gainRate.x = 8.0f;
		customFoveationParam.gainRate.y = 8.0f;
		customFoveationParam.areaSize = 1.0f;
		customFoveationParam.minResolution = 0.0625f;
		if (!qiyu_SetFoveation(foveatLevel_new, &customFoveationParam))
		{
			LOGE_("@@QYApp_testFoveation, fail to qiyu_SetFoveation(foveatLevel_new, &customFoveationParam)");
			return false;
		}
	}
	else
	{
		if (!qiyu_SetFoveation(foveatLevel_new))
		{
			LOGE_("@@QYApp_testFoveation, fail to qiyu_SetFoveation(foveatLevel_new)");
			return false;
		}
	}

	qiyu_FoveationLevel foveatLevel_log;
	qiyu_FoveationParam foveationParam_log;
	if (!qiyu_GetFoveation(foveatLevel_log, foveationParam_log))
	{
		LOGE_("@@QYApp_testFoveation, fail to qiyu_GetFoveation(foveatLevel_log, foveationParam_log)");
		return false;
	}
	LOGI_("$TEST$ change to FoveationLevel{%d}, FoveationParam{gainRate(%f, %f), areaSize(%f), minResolution(%f)}", foveatLevel_log, foveationParam_log.gainRate.x, foveationParam_log.gainRate.y, foveationParam_log.areaSize, foveationParam_log.minResolution);

	s_bFoveationChanged = true;
	return true;
}

static bool QYApp_InitEyeBuffers(int deviceW, int deviceH, int iMsaaSamples, int iColorFormat, bool bDepthBuffer, bool isProtectedContent)
{
	assert(s_iEyeTargetDeviceW == deviceW);
	assert(s_iEyeTargetDeviceH == deviceH);

	bool isSupport_Foveation = QY_GL_EXT::IsSupport_Foveation();
	static const bool isFoveationSubsampled = false;//FIXME! now fix false
	GLuint foveationFeatureBits = isFoveationSubsampled 
								? GL_FOVEATION_ENABLE_BIT_QCOM | GL_FOVEATION_SUBSAMPLED_LAYOUT_METHOD_BIT_QCOM 
								: GL_FOVEATION_ENABLE_BIT_QCOM | GL_FOVEATION_SCALED_BIN_METHOD_BIT_QCOM;

	for (int i = 0; i < EYE_BUFFERS_COUNT; ++i)
	{
		LOGI_("@@QYApp_InitEyeBuffers, Allocating Separate Single EyeBuffers");
		if (!qyEyeBuffers[i].qyRT[EYE_Left].Init(isSupport_Foveation, foveationFeatureBits, false, deviceW * s_fRTScale, deviceH * s_fRTScale, iMsaaSamples, iColorFormat, bDepthBuffer, isProtectedContent))
		{
			return false;
		}
		if (!qyEyeBuffers[i].qyRT[EYE_Right].Init(isSupport_Foveation, foveationFeatureBits, false, deviceW * s_fRTScale, deviceH * s_fRTScale, iMsaaSamples, iColorFormat, bDepthBuffer, isProtectedContent))
		{
			return false;
		}
		//
		if (!qyEyeBuffers[i].qyRTcustom[EYE_Left].Init(isSupport_Foveation, foveationFeatureBits, false, deviceW * s_fRTCustomScale, deviceH * s_fRTCustomScale, iMsaaSamples, iColorFormat, bDepthBuffer, isProtectedContent))
		{
			return false;
		}
		if (!qyEyeBuffers[i].qyRTcustom[EYE_Right].Init(isSupport_Foveation, foveationFeatureBits, false, deviceW * s_fRTCustomScale, deviceH * s_fRTCustomScale, iMsaaSamples, iColorFormat, bDepthBuffer, isProtectedContent))
		{
			return false;
		}

		LOGI_("@@QYApp_InitEyeBuffers, Allocating MultiView EyeBuffers");
		if (!qyEyeBuffers[i].qyMultiViewRT.Init(isSupport_Foveation, foveationFeatureBits, true, deviceW * s_fRTScale, deviceH * s_fRTScale, iMsaaSamples, iColorFormat, bDepthBuffer, isProtectedContent))
		{
			return false;
		}
		//
		if (!qyEyeBuffers[i].qyMultiViewRTcustom.Init(isSupport_Foveation, foveationFeatureBits, true, deviceW * s_fRTCustomScale, deviceH * s_fRTCustomScale, iMsaaSamples, iColorFormat, bDepthBuffer, isProtectedContent))
		{
			return false;
		}
	}
	return true;
}

static void QYApp_ReleaseEyeBuffers()
{
	for (int i = 0; i < EYE_BUFFERS_COUNT; ++i)
	{
		qyEyeBuffers[i].qyRT[EYE_Left].Release();
		qyEyeBuffers[i].qyRT[EYE_Right].Release();
		qyEyeBuffers[i].qyRTcustom[EYE_Left].Release();
		qyEyeBuffers[i].qyRTcustom[EYE_Right].Release();

		qyEyeBuffers[i].qyMultiViewRT.Release();
		qyEyeBuffers[i].qyMultiViewRTcustom.Release();
	}
}


static QYHandleRay* QYApp_InitHandleRay(const float radius, const int circleResolution)
{
	unsigned int numElementsPerVert = 3;
	unsigned int numAttribs = 1;
	int stride = (int)(numElementsPerVert * sizeof(float));
	QYHandleRay::QYProgramAttribute attribs[1] =
	{
		{0, 3, GL_FLOAT, false, stride, 0},//pos
	};
	int numVerts = circleResolution + 2;
	float thetaStep = M_PI * 2 / circleResolution;
	float *verts = new float[numVerts * numElementsPerVert];
	int vertIdx = 0;
	for (; vertIdx < circleResolution; ++vertIdx)
	{
		verts[3 * vertIdx] = radius * std::cos(vertIdx * thetaStep);
		verts[3 * vertIdx + 1] = radius * std::sin(vertIdx * thetaStep);
		verts[3 * vertIdx + 2] = 0;
	}

	int baseCenterVertIdx = vertIdx;
	verts[3 * vertIdx] = 0;
	verts[3 * vertIdx + 1] = 0;
	verts[3 * vertIdx + 2] = 0;
	++vertIdx;

	int topVertIdx = vertIdx;
	verts[3 * vertIdx] = 0;
	verts[3 * vertIdx + 1] = 0;
	verts[3 * vertIdx + 2] = 1.0f;

	int numTri = 2 * circleResolution;
	int numIndices = numTri * 3;
	int *indices = new int[numIndices];

	int indicesIdx = 0;
	for (int triIdx = 0; triIdx < circleResolution - 1; ++triIdx)
	{
		indices[indicesIdx++] = baseCenterVertIdx;
		indices[indicesIdx++] = triIdx + 1;
		indices[indicesIdx++] = triIdx;
	}
	indices[indicesIdx++] = baseCenterVertIdx;
	indices[indicesIdx++] = 0;
	indices[indicesIdx++] = circleResolution - 1;

	for (int triIdx = 0; triIdx < circleResolution - 1; ++triIdx)
	{
		indices[indicesIdx++] = topVertIdx;
		indices[indicesIdx++] = triIdx;
		indices[indicesIdx++] = triIdx + 1;
	}
	indices[indicesIdx++] = topVertIdx;
	indices[indicesIdx++] = circleResolution - 1;
	indices[indicesIdx++] = 0;

	QYHandleRay* pRay = new QYHandleRay(&attribs[0], numAttribs, (unsigned int *)indices, numIndices, verts, numElementsPerVert * numVerts * sizeof(float), numVerts);
	delete[] verts;
	delete[] indices;
	return pRay;
}

static bool QYApp_InitHandleLR(const QYApp* qyApp)
{
	if (g_pHandleL != nullptr && g_pHandleR != nullptr && g_pHandleShader != nullptr && g_pHandleShader_MultiView != nullptr)
		return false;

	char szHandleL[512];
	sprintf(szHandleL, "%s/%s", qyApp->szExternalPath, "L_handle.fbx");
	g_pHandleL = new QYModel(szHandleL);
	//
	char szHandleR[512];
	sprintf(szHandleR, "%s/%s", qyApp->szExternalPath, "R_handle.fbx");
	g_pHandleR = new QYModel(szHandleR);
	//
	//assert(g_pHandleL->HasTextures() && g_pHandleR->HasTextures());
	if (g_pHandleL->HasTextures() && g_pHandleR->HasTextures())
	{
		g_pHandleShader = new QYShader(shader_handle_vs, shader_handleTex_fs);
		g_pHandleShader_MultiView = new QYShader(shader_handle_vs_MultiView, shader_handleTex_fs_MultiView);
	}
	else
	{
		g_pHandleShader = new QYShader(shader_handle_vs, shader_handle_fs);
		g_pHandleShader_MultiView = new QYShader(shader_handle_vs_MultiView, shader_handle_fs_MultiView);
	}

	return true;
}

static bool QYApp_InitHandleRayLR()
{
	g_pHandleRayShader = new QYShader(shader_ray_vs, shader_ray_fs);
	g_pHandleRayShader_MultiView = new QYShader(shader_ray_vs_MultiView, shader_ray_fs_MultiView);
	g_pHandleRayLR = QYApp_InitHandleRay(0.003f, 20);
	return true;
}

static bool QYApp_InitTeapots(const QYApp* qyApp)
{
	char szTeapotPath[512];
	sprintf(szTeapotPath, "%s/%s", qyApp->szExternalPath, "teapot.fbx");
	g_pTeapot = new QYModel(szTeapotPath);
	g_pTeapotShader = new QYShader(shader_handle_vs, shader_handle_fs);
	g_pTeapotShader_MultiView = new QYShader(shader_handle_vs_MultiView, shader_handle_fs_MultiView);

	static const float s_testModelPos = 4.0f;
	static const float s_testModelPos_Yoffset = 1.0f;
	float colorScale = 0.8f;
	s_teapotModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(s_testModelPos,	0.0f - s_testModelPos_Yoffset,				0.0f));
	s_teapotColor[0] = colorScale * glm::vec3(1.0f, 0.0f, 0.0f);

	s_teapotModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,			s_testModelPos - s_testModelPos_Yoffset,	0.0f));
	s_teapotColor[1] = colorScale * glm::vec3(0.0f, 1.0f, 0.0f);

	s_teapotModelMatrix[2] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,			0.0f - s_testModelPos_Yoffset,				s_testModelPos));
	s_teapotColor[2] = colorScale * glm::vec3(0.0f, 0.0f, 1.0f);

	s_teapotModelMatrix[3] = glm::translate(glm::mat4(1.0f), glm::vec3(-s_testModelPos, 0.0f - s_testModelPos_Yoffset,				0.0f));
	s_teapotColor[3] = colorScale * glm::vec3(0.0f, 1.0f, 1.0f);

	s_teapotModelMatrix[4] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,			-s_testModelPos - s_testModelPos_Yoffset,	0.0f));
	s_teapotColor[4] = colorScale * glm::vec3(1.0f, 0.0f, 1.0f);

	s_teapotModelMatrix[5] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,			0.0f - s_testModelPos_Yoffset,				-s_testModelPos));
	s_teapotColor[5] = colorScale * glm::vec3(1.0f, 1.0f, 0.0f);
	s_teapotModelMatrix[6] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,			1.2f - s_testModelPos_Yoffset,				-s_testModelPos));
	s_teapotColor[6] = colorScale * glm::vec3(1.0f, 1.0f, 0.0f);

	return true;
}

static const char* EglErrorString(const EGLint error)
{
	switch (error)
	{
	case EGL_SUCCESS:
		return "EGL_SUCCESS";
	case EGL_NOT_INITIALIZED:
		return "EGL_NOT_INITIALIZED";
	case EGL_BAD_ACCESS:
		return "EGL_BAD_ACCESS";
	case EGL_BAD_ALLOC:
		return "EGL_BAD_ALLOC";
	case EGL_BAD_ATTRIBUTE:
		return "EGL_BAD_ATTRIBUTE";
	case EGL_BAD_CONTEXT:
		return "EGL_BAD_CONTEXT";
	case EGL_BAD_CONFIG:
		return "EGL_BAD_CONFIG";
	case EGL_BAD_CURRENT_SURFACE:
		return "EGL_BAD_CURRENT_SURFACE";
	case EGL_BAD_DISPLAY:
		return "EGL_BAD_DISPLAY";
	case EGL_BAD_SURFACE:
		return "EGL_BAD_SURFACE";
	case EGL_BAD_MATCH:
		return "EGL_BAD_MATCH";
	case EGL_BAD_PARAMETER:
		return "EGL_BAD_PARAMETER";
	case EGL_BAD_NATIVE_PIXMAP:
		return "EGL_BAD_NATIVE_PIXMAP";
	case EGL_BAD_NATIVE_WINDOW:
		return "EGL_BAD_NATIVE_WINDOW";
	case EGL_CONTEXT_LOST:
		return "EGL_CONTEXT_LOST";
	default:
		return "unknown";
	}
}

static bool QYApp_CreateContext(QYEgl* egl, const QYEgl* shareEgl)
{
	egl->eglMajorVersion = 0;
	egl->eglMinorVersion = 0;
	egl->eglDisplay = 0;
	egl->eglConfig = 0;
	egl->eglSurface = EGL_NO_SURFACE;
	egl->eglContext = EGL_NO_CONTEXT;

	egl->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(egl->eglDisplay, &egl->eglMajorVersion, &egl->eglMinorVersion);

	const int MAX_CONFIGS = 1024;
	EGLConfig configs[MAX_CONFIGS];
	EGLint numConfigs = 0;
	if (eglGetConfigs(egl->eglDisplay, configs, MAX_CONFIGS, &numConfigs) == EGL_FALSE)
	{
		LOGE_("@@QYApp_CreateContext, fail to eglGetConfigs: %s", EglErrorString(eglGetError()));
		return false;
	}
	const EGLint configAttribs[] =
	{
		EGL_RED_SIZE,
		8,
		EGL_GREEN_SIZE,
		8,
		EGL_BLUE_SIZE,
		8,
		EGL_ALPHA_SIZE,
		8,
		EGL_DEPTH_SIZE,
		0,
		EGL_STENCIL_SIZE,
		0,
		EGL_SAMPLES,
		0,
		EGL_NONE
	};
	egl->eglConfig = 0;
	for (int i = 0; i < numConfigs; i++)
	{
		EGLint value = 0;
		eglGetConfigAttrib(egl->eglDisplay, configs[i], EGL_RENDERABLE_TYPE, &value);
		if ((value & EGL_OPENGL_ES3_BIT_KHR) != EGL_OPENGL_ES3_BIT_KHR)
		{
			continue;
		}

		eglGetConfigAttrib(egl->eglDisplay, configs[i], EGL_SURFACE_TYPE, &value);
		if ((value & (EGL_WINDOW_BIT | EGL_PBUFFER_BIT)) != (EGL_WINDOW_BIT | EGL_PBUFFER_BIT))
		{
			continue;
		}
		int j = 0;
		for (; configAttribs[j] != EGL_NONE; j += 2)
		{
			eglGetConfigAttrib(egl->eglDisplay, configs[i], configAttribs[j], &value);
			if (value != configAttribs[j + 1])
			{
				break;
			}
		}
		if (configAttribs[j] == EGL_NONE)
		{
			egl->eglConfig = configs[i];
			break;
		}
	}
	if (egl->eglConfig == 0)
	{
		LOGE_("@@QYApp_CreateContext, fail to eglChooseConfig: %s", EglErrorString(eglGetError()));
		return false;
	}
	EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
	egl->eglContext = eglCreateContext(egl->eglDisplay, egl->eglConfig, (shareEgl != NULL) ? shareEgl->eglContext : EGL_NO_CONTEXT, contextAttribs);
	if (egl->eglContext == EGL_NO_CONTEXT)
	{
		LOGE_("@@QYApp_CreateContext, fail to eglCreateContext: %s", EglErrorString(eglGetError()));
		return false;
	}
	const EGLint surfaceAttribs[] = { EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE };
	egl->eglSurface = eglCreatePbufferSurface(egl->eglDisplay, egl->eglConfig, surfaceAttribs);
	if (egl->eglSurface == EGL_NO_SURFACE)
	{
		LOGE_("@@QYApp_CreateContext, fail to eglCreatePbufferSurface: %s", EglErrorString(eglGetError()));
		eglDestroyContext(egl->eglDisplay, egl->eglContext);
		egl->eglContext = EGL_NO_CONTEXT;
		return false;
	}

	if (eglMakeCurrent(egl->eglDisplay, egl->eglSurface, egl->eglSurface, egl->eglContext) == EGL_FALSE)
	{
		LOGE_("@@QYApp_CreateContext, fail to eglMakeCurrent: %s", EglErrorString(eglGetError()));
		eglDestroySurface(egl->eglDisplay, egl->eglSurface);
		eglDestroyContext(egl->eglDisplay, egl->eglContext);
		egl->eglContext = EGL_NO_CONTEXT;
		return false;
	}

	return true;
}

static void QYApp_DestroyContext(QYEgl* egl)
{
	if (egl->eglDisplay != 0)
	{
		LOGE_("@@QYApp_DestroyContext, eglMakeCurrent( Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT )");
		if (eglMakeCurrent(egl->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_FALSE)
		{
			LOGE_("@@QYApp_DestroyContext, eglMakeCurrent() failed: %s", EglErrorString(eglGetError()));
		}
	}
	if (egl->eglContext != EGL_NO_CONTEXT)
	{
		LOGE_("@@QYApp_DestroyContext, eglDestroyContext( Display, Context )");
		if (eglDestroyContext(egl->eglDisplay, egl->eglContext) == EGL_FALSE)
		{
			LOGE_("@@QYApp_DestroyContext, eglDestroyContext() failed: %s", EglErrorString(eglGetError()));
		}
		egl->eglContext = EGL_NO_CONTEXT;
	}
	if (egl->eglSurface != EGL_NO_SURFACE)
	{
		LOGE_("@@QYApp_DestroyContext, eglDestroySurface( Display, eglSurface )");
		if (eglDestroySurface(egl->eglDisplay, egl->eglSurface) == EGL_FALSE)
		{
			LOGE_("@@QYApp_DestroyContext, eglDestroySurface() failed: %s", EglErrorString(eglGetError()));
		}
		egl->eglSurface = EGL_NO_SURFACE;
	}
	if (egl->eglDisplay != 0)
	{
		LOGE_("@@QYApp_DestroyContext, eglTerminate( Display )");
		if (eglTerminate(egl->eglDisplay) == EGL_FALSE)
		{
			LOGE_("@@QYApp_DestroyContext, eglTerminate() failed: %s", EglErrorString(eglGetError()));
		}
		egl->eglDisplay = 0;
	}
}

static bool QYApp_Init(QYApp* qyApp)
{
	if (g_isInited)//
	{
		LOGE_("@@QYApp_Init, already init");
		return false;
	}

	if (!QY_GL_EXT::InitFunction_MultiView())
	{
		LOGE_("@@QYApp_Init, fail to QY_GL_EXT::InitFunction_MultiView");
		return false;
	}
	if (!QY_GL_EXT::InitFunction_Foveation())
	{
		LOGE_("@@QYApp_Init, fail to QY_GL_EXT::InitFunction_Foveation");
		return false;
	}

	if (!qiyu_Init(qyApp->pANativeActivity->clazz,
					qyApp->pANativeActivity->vm,
					qiyu_GraphicsApi::GA_OpenGLES,
					qiyu_TrackingOriginMode::TM_Device,
					false))
	{
		LOGE_("@@QYApp_Init, fail to qiyu_Init");
		return false;//!
	}
	qiyu_SdkVersion sdkVersion = qiyu_GetSdkVersion();
	LOGI_("@@QYApp_Init, qiyu native sdk version : %d.%d.%d", sdkVersion.versionProduct, sdkVersion.versionMajor, sdkVersion.versionMinor);
	qiyu_DeviceInfo deviceInfo = qiyu_GetDeviceInfo();
	LOGI_("@@QYApp_Init, deviceInfo.iEyeTargetWidth : %d, deviceInfo.iEyeTargetHeight : %d", deviceInfo.iEyeTargetWidth, deviceInfo.iEyeTargetHeight);
	QYApp_InitEyeTargetSize(deviceInfo.iEyeTargetWidth, deviceInfo.iEyeTargetHeight);

	if (!QYApp_CreateContext(&qyApp->qyEgl, NULL))
	{
		LOGE_("@@QYApp_Init, fail to QYApp_CreateContext");
		QYApp_DestroyContext(&qyApp->qyEgl);
		return false;
	}

	if (!QYApp_InitEyeBuffers(deviceInfo.iEyeTargetWidth, deviceInfo.iEyeTargetHeight, s_iMsaaSamples, GL_RGBA8, true, s_bIsProtectedContent))
	{
		LOGE_("@@QYApp_Init, fail to QYApp_InitEyeBuffers");
		QYApp_ReleaseEyeBuffers();
		return false;
	}

	if (!QYApp_InitTeapots(qyApp))
	{
		LOGE_("@@QYApp_Init, fail to QYApp_InitTeapots");
		return false;
	}

	if (!QYApp_InitHandleLR(qyApp))
	{
		LOGE_("@@QYApp_Init, fail to QYApp_InitHandleLR");
		return false;
	}
	if (!QYApp_InitHandleRayLR())
	{
		LOGE_("@@QYApp_Init, fail to QYApp_InitHandleRayLR");
		return false;
	}

	//if (!g_pQYFmod->Init(qyApp->szExternalPath))
	//{
	//	LOGE_("@@QYApp_Init, fail to g_pQYFmod->Init");
	//	return false;
	//}

	g_isInited = true;
	return true;
}

static void RenderHandleRayLR(bool bMultiView, bool bTrigger, qiyu_Eye eyeType, const glm::vec3& rayStartPosInWorld, const glm::vec3& rayDirectionInWorld)
{
	QYShader* curShader = bMultiView ? g_pHandleRayShader_MultiView : g_pHandleRayShader;
	if (!curShader->Bind())
	{
		LOGE_("@@RenderHandleRayLR, fail to curShader->Bind, bMultiView: %d", bMultiView);
		assert(0 && "@@RenderHandleRayLR, fail to curShader->Bind");
	}

	if (bMultiView)
	{
		curShader->SetMatrix4("u_matProj_ray", g_matProj[0]);
		curShader->SetMat4fv("u_matView_ray[0]", 2, glm::value_ptr(g_matView[0][0]));
	}
	else
	{
		curShader->SetMatrix4("u_matProj_ray", g_matProj[eyeType]);
		curShader->SetMatrix4("u_matView_ray", g_matView[eyeType]);
	}

	glm::vec3 rayEndPosInWorld = rayStartPosInWorld + rayDirectionInWorld * 10.0f/*MaxDistance*/;

	glm::vec3 directionVector = rayEndPosInWorld - rayStartPosInWorld;
	float rayLength = glm::length(directionVector);

	glm::fquat rotation = glm::rotation({ 0, 0, 1 }, glm::normalize(directionVector));
	glm::mat4 modelMat = glm::mat4_cast(rotation);
	modelMat[3] = glm::vec4(rayStartPosInWorld, 1.0f);
	modelMat = glm::scale(modelMat, glm::vec3(1.0f, 1.0f, rayLength));

	static glm::vec4 s_clrRay_trigger = glm::vec4(34.0f, 70.0f, 180.0f, 200.0f) / 255.0f;
	static glm::vec4 s_clrRay_default = glm::vec4(180.0f, 180.0f, 180.0f, 200.0f) / 255.0f;

	curShader->SetMatrix4("u_matModel_ray", modelMat);
	curShader->SetVector4("u_clrRay", bTrigger ? s_clrRay_trigger : s_clrRay_default);

	assert(g_pHandleRayLR);
	g_pHandleRayLR->Draw();

	if (!curShader->UnBind())
	{
		LOGE_("@@RenderHandleRayLR, fail to curShader->UnBind, bMultiView: %d", bMultiView);
		assert(0 && "@@RenderHandleRayLR, fail to curShader->UnBind");
	}
}

static void RenderHandleLR(bool bMultiView, qiyu_Eye eyeType, const ControllerState& ctrlState)
{
	if (ctrlState.bControllerIsInit)
	{
		QYShader* curShader = bMultiView ? g_pHandleShader_MultiView : g_pHandleShader;
		if (ctrlState.qyCtrlData[CI_Left].isConnect && ctrlState.qyCtrlData[CI_Left].isShow)
		{
			if (!curShader->Bind())
			{
				LOGE_("@@RenderHandleLR, fail to curShader->Bind, leftHandle, bMultiView: %d", bMultiView);
				assert(0 && "@@RenderHandleLR, fail to curShader->Bind, leftHandle");
			}
			if (bMultiView)
			{
				g_pHandleL->DrawHandleLR(bMultiView, g_fTrackingOffset, g_matProj[0], g_matView[0], curShader, ctrlState.qyCtrlData[CI_Left], false);
			}
			else
			{
				g_pHandleL->DrawHandleLR(bMultiView, g_fTrackingOffset, g_matProj[eyeType], g_matView[eyeType], curShader, ctrlState.qyCtrlData[CI_Left], false);
			}
			if (!curShader->UnBind())
			{
				LOGE_("@@RenderHandleLR, fail to curShader->UnBind, leftHandle, bMultiView: %d", bMultiView);
				assert(0 && "@@RenderHandleLR, fail to curShader->UnBind, leftHandle");
			}

			RenderHandleRayLR(bMultiView, (ctrlState.qyCtrlData[CI_Left].triggerForce > 0.0f), eyeType, g_pHandleL->GetRayStartWorldPos(), g_pHandleL->GetRayWorldDir());//triggerForce > 0.0f
		}
		if (ctrlState.qyCtrlData[CI_Right].isConnect && ctrlState.qyCtrlData[CI_Right].isShow)
		{
			if (!curShader->Bind())
			{
				LOGE_("@@RenderHandleLR, fail to curShader->Bind, rightHandle, bMultiView: %d", bMultiView);
				assert(0 && "@@RenderHandleLR, fail to curShader->Bind, rightHandle");
			}
			if (bMultiView)
			{
				g_pHandleR->DrawHandleLR(bMultiView, g_fTrackingOffset, g_matProj[0], g_matView[0], curShader, ctrlState.qyCtrlData[CI_Right], true);
			}
			else
			{
				g_pHandleR->DrawHandleLR(bMultiView, g_fTrackingOffset, g_matProj[eyeType], g_matView[eyeType], curShader, ctrlState.qyCtrlData[CI_Right], true);
			}
			if (!curShader->UnBind())
			{
				LOGE_("@@RenderHandleLR, fail to curShader->UnBind, rightHandle, bMultiView: %d", bMultiView);
				assert(0 && "@@RenderHandleLR, fail to curShader->UnBind, rightHandle");
			}

			RenderHandleRayLR(bMultiView, (ctrlState.qyCtrlData[CI_Right].triggerForce > 0.0f), eyeType, g_pHandleR->GetRayStartWorldPos(), g_pHandleR->GetRayWorldDir());//triggerForce > 0.0f
		}
	}
}

static void QYApp_RenderEye(bool bMultiView, bool bIsCustomScaleRT, qiyu_Eye eyeType, const ControllerState& ctrlState)
{
	QY_GL(glEnable(GL_SCISSOR_TEST));
	QY_GL(glEnable(GL_DEPTH_TEST));
	QY_GL(glDepthFunc(GL_LESS));
	QY_GL(glDepthMask(GL_TRUE));

	QYRenderTarget* curRT = nullptr;
	if (bIsCustomScaleRT)
	{
		if (bMultiView)
			curRT = &qyEyeBuffers[s_idxEyeBuffer].qyMultiViewRTcustom;
		else
			curRT = &qyEyeBuffers[s_idxEyeBuffer].qyRTcustom[eyeType];
	}
	else
	{
		if (bMultiView)
			curRT = &qyEyeBuffers[s_idxEyeBuffer].qyMultiViewRT;
		else
			curRT = &qyEyeBuffers[s_idxEyeBuffer].qyRT[eyeType];
	}

	if (QY_GL_EXT::IsSupport_Foveation())
	{
		GLuint focalPoint = 0;
		GLfloat focalX = 0.0f;
		GLfloat focalY = 0.0f;
		qiyu_FoveationLevel foveatLevel;
		qiyu_FoveationParam foveationParam;
		if (!qiyu_GetFoveation(foveatLevel, foveationParam))
		{
			LOGE_("@@QYApp_RenderEye, fail to qiyu_GetFoveation");
		}
		else
		{
			//LOGV_("@@QYApp_RenderEye, FoveationLevel{%d}, FoveationParam{gainRate(%f, %f), areaSize(%f), minResolution(%f)}", foveatLevel, foveationParam.gainRate.x, foveationParam.gainRate.y, foveationParam.areaSize, foveationParam.minResolution);
			if (bMultiView)
			{
				QY_GL(glTextureFoveationParametersQCOM(curRT->GetColorAttachment(), /*layer*/0, focalPoint, focalX, focalY, foveationParam.gainRate.x, foveationParam.gainRate.y, foveationParam.areaSize));//left eye
				QY_GL(glTextureFoveationParametersQCOM(curRT->GetColorAttachment(), /*layer*/1, focalPoint, focalX, focalY, foveationParam.gainRate.x, foveationParam.gainRate.y, foveationParam.areaSize));//right eye
			}
			else
			{
				QY_GL(glTextureFoveationParametersQCOM(curRT->GetColorAttachment(), /*layer*/0, focalPoint, focalX, focalY, foveationParam.gainRate.x, foveationParam.gainRate.y, foveationParam.areaSize));
			}

			if (s_bFoveationChanged)//
			{
				GLenum texType_ = bMultiView ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
				QY_GL(glBindTexture(texType_, curRT->GetColorAttachment()));
				//glTexParameteri(texType_, GL_TEXTURE_PREVIOUS_SOURCE_TEXTURE_QCOM, PreviousTexture);//FIXME! //TODO!
				glTexParameterf(texType_, GL_TEXTURE_FOVEATED_MIN_PIXEL_DENSITY_QCOM, foveationParam.minResolution);//FIXME! better set when create; otherwise is just for TEST!
				QY_GL(glBindTexture(texType_, 0));
			}
		}
	}

	if (!curRT->Bind())
	{
		LOGE_("@@QYApp_RenderEye, fail to curRT->Bind, s_idxEyeBuffer(%d), eyeType(%d), bIsCustomScaleRT(%d), bMultiView(%d)", s_idxEyeBuffer, eyeType, bIsCustomScaleRT, bMultiView);
		assert(0 && "@@QYApp_RenderEye, fail to curRT->Bind");
	}

	QY_GL(glViewport(0, 0, s_iEyeTargetScaledW, s_iEyeTargetScaledH));//changed by QYApp_switchEyeTargetSize
	QY_GL(glScissor(0, 0, s_iEyeTargetScaledW, s_iEyeTargetScaledH));//changed by QYApp_switchEyeTargetSize
	QY_GL(glClearColor(0.2f, 0.2f, 0.2f, 1.0f));
	QY_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	qiyu_StartEye(bMultiView, eyeType, (bMultiView ? TT_TextureArray : TT_Texture));//

	QYShader* curShader = bMultiView ? g_pTeapotShader_MultiView : g_pTeapotShader;
	if (!curShader->Bind())
	{
		LOGE_("@@QYApp_RenderEye, fail to curShader->Bind, bMultiView: %d", bMultiView);
		assert(0 && "@@QYApp_RenderEye, fail to curShader->Bind");
	}
	for (int j = 0; j < s_iTeapotCount; j++)
	{
		if (s_bRotateTeapots)
		{
			s_teapotModelMatrix[j] = glm::rotate(s_teapotModelMatrix[j], glm::radians(s_fRotateAmount), glm::vec3(0.0f, 1.0f, 0.0f));
		}

		static float fix_fScale = 0.05f;//0.001f
		static glm::mat4 fix_matScale = glm::scale(glm::mat4(1.0f), glm::vec3(fix_fScale));
		glm::mat4 matWorld = s_teapotModelMatrix[j] * fix_matScale;

		curShader->SetMatrix4("u_matModel", matWorld);
		if (bMultiView)
		{
			curShader->SetMatrix4("u_matProj", g_matProj[0]);
			curShader->SetMat4fv("u_matView[0]", 2, glm::value_ptr(g_matView[0][0]));
		}
		else
		{
			curShader->SetMatrix4("u_matProj", g_matProj[eyeType]);
			curShader->SetMatrix4("u_matView", g_matView[eyeType]);
		}

		curShader->SetVector3("u_lightPos", glm::vec3(0.0f, 0.0f, 0.0f));
		curShader->SetVector3("u_clrLight", s_teapotColor[j]);

		curShader->SetVector3("u_viewPos", glm::vec3(0, 0, 0));

		g_pTeapot->DrawTeapot(curShader);
	}
	glm::mat4 mat_cube;
	for (int c = 0; c < g_pointsBoundaryGeometry.size(); c++)
	{
		static float fix_fScale = 0.002f;//0.001f
		static glm::mat4 fix_matScale = glm::scale(glm::mat4(1.0f), glm::vec3(fix_fScale));
		const qiyu_Vector3& cur_cubePos = g_pointsBoundaryGeometry.at(c);
		mat_cube = glm::translate(glm::mat4(1.0f), glm::vec3(cur_cubePos.x, cur_cubePos.y, cur_cubePos.z));
		glm::mat4 matWorld = mat_cube * fix_matScale;

		curShader->SetMatrix4("u_matModel", matWorld);
		if (bMultiView)
		{
			curShader->SetMatrix4("u_matProj", g_matProj[0]);
			curShader->SetMat4fv("u_matView[0]", 2, glm::value_ptr(g_matView[0][0]));
		}
		else
		{
			curShader->SetMatrix4("u_matProj", g_matProj[eyeType]);
			curShader->SetMatrix4("u_matView", g_matView[eyeType]);
		}

		curShader->SetVector3("u_lightPos", glm::vec3(0.0f, 0.0f, 0.0f));
		curShader->SetVector3("u_clrLight", glm::vec3(1.0f, 1.0f, 1.0f));

		curShader->SetVector3("u_viewPos", glm::vec3(0, 0, 0));

		g_pTeapot->DrawTeapot(curShader);
	}
	if (!curShader->UnBind())
	{
		LOGE_("@@QYApp_RenderEye, fail to curShader->UnBind, bMultiView: %d", bMultiView);
		assert(0 && "@@QYApp_RenderEye, fail to curShader->UnBind");
	}

	RenderHandleLR(bMultiView, eyeType, ctrlState);

	qiyu_EndEye(bMultiView, eyeType, (bMultiView ? TT_TextureArray : TT_Texture));//

	if (!curRT->UnBind())
	{
		LOGE_("@@QYApp_RenderEye, fail to curRT->UnBind, s_idxEyeBuffer(%d), eyeType(%d), bIsCustomScaleRT(%d), bMultiView(%d)", s_idxEyeBuffer, eyeType, bIsCustomScaleRT, bMultiView);
		assert(0 && "@@QYApp_RenderEye, fail to curRT->UnBind");
	}
}

static void CreateLayout_(float centerX, float centerY, float radiusX, float radiusY, qiyu_RenderLayer_ScreenPosUV* pLayout)//FIXME! //TODO!
{
	// This is always in screen space so we want Z = 0 and W = 1
	float lowerLeftPos[4] = { centerX - radiusX, centerY - radiusY, 0.0f, 1.0f };
	float lowerRightPos[4] = { centerX + radiusX, centerY - radiusY, 0.0f, 1.0f };
	float upperLeftPos[4] = { centerX - radiusX, centerY + radiusY, 0.0f, 1.0f };
	float upperRightPos[4] = { centerX + radiusX, centerY + radiusY, 0.0f, 1.0f };
	float lowerUVs[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	float upperUVs[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
	memcpy(pLayout->LowerLeftPos, lowerLeftPos, sizeof(lowerLeftPos));
	memcpy(pLayout->LowerRightPos, lowerRightPos, sizeof(lowerRightPos));
	memcpy(pLayout->UpperLeftPos, upperLeftPos, sizeof(upperLeftPos));
	memcpy(pLayout->UpperRightPos, upperRightPos, sizeof(upperRightPos));
	memcpy(pLayout->LowerUVs, lowerUVs, sizeof(lowerUVs));
	memcpy(pLayout->UpperUVs, upperUVs, sizeof(upperUVs));
}
static bool QYApp_Render(bool bMultiView, bool bIsCustomScaleRT, const ControllerState& ctrlState)
{
	assert(qiyu_GetGraphicsApi() == qiyu_GraphicsApi::GA_OpenGLES);
	if (bMultiView)
	{
		QYApp_RenderEye(bMultiView, bIsCustomScaleRT, EYE_Left, ctrlState);
		if (bIsCustomScaleRT)
		{
			qiyu_FrameParam frameParam;
			memset(&frameParam, 0, sizeof(frameParam));//
			frameParam.minVsyncs = 1;
			frameParam.headPoseState = g_headPoseState;
			//!
			frameParam.renderLayers[0].imageHandle = qyEyeBuffers[s_idxEyeBuffer].qyMultiViewRTcustom.GetColorAttachment();
			frameParam.renderLayers[0].imageType = TT_TextureArray;//!!
			CreateLayout_(0.0f, 0.0f, 1.0f, 1.0f, &frameParam.renderLayers[0].imageCoords);//FIXME! //TODO!
			frameParam.renderLayers[0].eyeMask = RL_EyeMask_Left;//!
			//
			frameParam.renderLayers[1].imageHandle = qyEyeBuffers[s_idxEyeBuffer].qyMultiViewRTcustom.GetColorAttachment();
			frameParam.renderLayers[1].imageType = TT_TextureArray;//!!
			CreateLayout_(0.0f, 0.0f, 1.0f, 1.0f, &frameParam.renderLayers[1].imageCoords);//FIXME! //TODO!
			frameParam.renderLayers[1].eyeMask = RL_EyeMask_Right;//!
			//!
			qiyu_SubmitFrame(frameParam);
		}
		else
		{
			qiyu_FrameParam frameParam;
			memset(&frameParam, 0, sizeof(frameParam));//
			frameParam.minVsyncs = 1;
			frameParam.headPoseState = g_headPoseState;
			//!
			frameParam.renderLayers[0].imageHandle = qyEyeBuffers[s_idxEyeBuffer].qyMultiViewRT.GetColorAttachment();
			frameParam.renderLayers[0].imageType = TT_TextureArray;//!!
			CreateLayout_(0.0f, 0.0f, 1.0f, 1.0f, &frameParam.renderLayers[0].imageCoords);//FIXME! //TODO!
			frameParam.renderLayers[0].eyeMask = RL_EyeMask_Left;//!
			//
			frameParam.renderLayers[1].imageHandle = qyEyeBuffers[s_idxEyeBuffer].qyMultiViewRT.GetColorAttachment();
			frameParam.renderLayers[1].imageType = TT_TextureArray;//!!
			CreateLayout_(0.0f, 0.0f, 1.0f, 1.0f, &frameParam.renderLayers[1].imageCoords);//FIXME! //TODO!
			frameParam.renderLayers[1].eyeMask = RL_EyeMask_Right;//!
			//!
			qiyu_SubmitFrame(frameParam);
		}
	}
	else
	{
		QYApp_RenderEye(bMultiView, bIsCustomScaleRT, EYE_Left, ctrlState);
		QYApp_RenderEye(bMultiView, bIsCustomScaleRT, EYE_Right, ctrlState);
		if (bIsCustomScaleRT)
		{
			qiyu_FrameParam frameParam;
			memset(&frameParam, 0, sizeof(frameParam));//
			frameParam.minVsyncs = 1;
			frameParam.headPoseState = g_headPoseState;
			//!
			frameParam.renderLayers[0].imageHandle = qyEyeBuffers[s_idxEyeBuffer].qyRTcustom[EYE_Left].GetColorAttachment();
			frameParam.renderLayers[0].imageType = TT_Texture;//!!
			CreateLayout_(0.0f, 0.0f, 1.0f, 1.0f, &frameParam.renderLayers[0].imageCoords);//FIXME! //TODO!
			frameParam.renderLayers[0].eyeMask = RL_EyeMask_Left;//!
			//
			frameParam.renderLayers[1].imageHandle = qyEyeBuffers[s_idxEyeBuffer].qyRTcustom[EYE_Right].GetColorAttachment();
			frameParam.renderLayers[1].imageType = TT_Texture;//!!
			CreateLayout_(0.0f, 0.0f, 1.0f, 1.0f, &frameParam.renderLayers[1].imageCoords);//FIXME! //TODO!
			frameParam.renderLayers[1].eyeMask = RL_EyeMask_Right;//!
			//!
			qiyu_SubmitFrame(frameParam);
		}
		else
		{
			qiyu_FrameParam frameParam;
			memset(&frameParam, 0, sizeof(frameParam));//
			frameParam.minVsyncs = 1;
			frameParam.headPoseState = g_headPoseState;
			//!
			frameParam.renderLayers[0].imageHandle = qyEyeBuffers[s_idxEyeBuffer].qyRT[EYE_Left].GetColorAttachment();
			frameParam.renderLayers[0].imageType = TT_Texture;//!!
			CreateLayout_(0.0f, 0.0f, 1.0f, 1.0f, &frameParam.renderLayers[0].imageCoords);//FIXME! //TODO!
			frameParam.renderLayers[0].eyeMask = RL_EyeMask_Left;//!
			//
			frameParam.renderLayers[1].imageHandle = qyEyeBuffers[s_idxEyeBuffer].qyRT[EYE_Right].GetColorAttachment();
			frameParam.renderLayers[1].imageType = TT_Texture;//!!
			CreateLayout_(0.0f, 0.0f, 1.0f, 1.0f, &frameParam.renderLayers[1].imageCoords);//FIXME! //TODO!
			frameParam.renderLayers[1].eyeMask = RL_EyeMask_Right;//!
			//!
			qiyu_SubmitFrame(frameParam);
		}
	}

	s_idxEyeBuffer = (s_idxEyeBuffer + 1) % EYE_BUFFERS_COUNT;

	return true;
}

static bool QYApp_PreRender()
{
	qiyu_DeviceInfo di = qiyu_GetDeviceInfo();
	float fPredictedTimeMs = qiyu_PredictDisplayTime();
	g_headPoseState = qiyu_PredictHeadPose(fPredictedTimeMs);

	//qiyu_Vector3 leftEyePos;
	//leftEyePos.x = di.frustumLeftEye.position.x;
	//leftEyePos.y = di.frustumLeftEye.position.y;
	//leftEyePos.z = di.frustumLeftEye.position.z;
	//qiyu_Vector3 rightEyePos;
	//rightEyePos.x = di.frustumRightEye.position.x;
	//rightEyePos.y = di.frustumRightEye.position.y;
	//rightEyePos.z = di.frustumRightEye.position.z;
	qiyu_Quaternion leftEyeRot;// glm quat is (w)(xyz), BUT here is xyzw
	leftEyeRot.x = di.frustumLeftEye.rotation.x;
	leftEyeRot.y = di.frustumLeftEye.rotation.y;
	leftEyeRot.z = di.frustumLeftEye.rotation.z;
	leftEyeRot.w = di.frustumLeftEye.rotation.w;
	qiyu_Quaternion rightEyeRot;// glm quat is (w)(xyz), BUT here is xyzw
	rightEyeRot.x = di.frustumRightEye.rotation.x;
	rightEyeRot.y = di.frustumRightEye.rotation.y;
	rightEyeRot.z = di.frustumRightEye.rotation.z;
	rightEyeRot.w = di.frustumRightEye.rotation.w;
	//LOGV_("@@QYApp_PreRender, leftEyePos(%f, %f, %f), rightEyePos(%f, %f, %f), leftEyeRot(%f, %f, %f, %f), rightEyeRot(%f, %f, %f, %f)", leftEyePos.x, leftEyePos.y, leftEyePos.z, rightEyePos.x, rightEyePos.y, rightEyePos.z, leftEyeRot.x, leftEyeRot.y, leftEyeRot.z, leftEyeRot.w, rightEyeRot.x, rightEyeRot.y, rightEyeRot.z, rightEyeRot.w);
	qiyu_Matrix4 outLeftEyeMatrix;
	qiyu_Matrix4 outRightEyeMatrix;
	if (!qiyu_GetViewMatrix(outLeftEyeMatrix, outRightEyeMatrix, g_fTrackingOffset, g_headPoseState, leftEyeRot, rightEyeRot))
	{
		LOGE_("@@QYApp_PreRender, fail to qiyu_GetViewMatrix");
	}
	g_matView[EYE_Left] = glm::mat4(outLeftEyeMatrix.M[0][0], outLeftEyeMatrix.M[0][1], outLeftEyeMatrix.M[0][2], outLeftEyeMatrix.M[0][3],
		outLeftEyeMatrix.M[1][0], outLeftEyeMatrix.M[1][1], outLeftEyeMatrix.M[1][2], outLeftEyeMatrix.M[1][3],
		outLeftEyeMatrix.M[2][0], outLeftEyeMatrix.M[2][1], outLeftEyeMatrix.M[2][2], outLeftEyeMatrix.M[2][3],
		outLeftEyeMatrix.M[3][0], outLeftEyeMatrix.M[3][1], outLeftEyeMatrix.M[3][2], outLeftEyeMatrix.M[3][3]);//FIXME!
	g_matView[EYE_Right] = glm::mat4(outRightEyeMatrix.M[0][0], outRightEyeMatrix.M[0][1], outRightEyeMatrix.M[0][2], outRightEyeMatrix.M[0][3],
		outRightEyeMatrix.M[1][0], outRightEyeMatrix.M[1][1], outRightEyeMatrix.M[1][2], outRightEyeMatrix.M[1][3],
		outRightEyeMatrix.M[2][0], outRightEyeMatrix.M[2][1], outRightEyeMatrix.M[2][2], outRightEyeMatrix.M[2][3],
		outRightEyeMatrix.M[3][0], outRightEyeMatrix.M[3][1], outRightEyeMatrix.M[3][2], outRightEyeMatrix.M[3][3]);//FIXME!

	qiyu_ViewFrustum* pLeftFrust = &di.frustumLeftEye;
	g_matProj[EYE_Left] = glm::frustum(pLeftFrust->left, pLeftFrust->right, pLeftFrust->bottom, pLeftFrust->top, pLeftFrust->near, pLeftFrust->far);
	qiyu_ViewFrustum* pRightFrust = &di.frustumRightEye;
	g_matProj[EYE_Right] = glm::frustum(pRightFrust->left, pRightFrust->right, pRightFrust->bottom, pRightFrust->top, pRightFrust->near, pRightFrust->far);

	return true;
}

static bool QYApp_PostRender()
{
	if (s_bFoveationChanged/* && QY_GL_EXT::IsSupport_Foveation()*/)
	{
		s_bFoveationChanged = false;
	}
	
	return true;
}

static unsigned int GetCurTime()//in milliseconds
{
	timeval t;
	t.tv_sec = t.tv_usec = 0;
	if (gettimeofday(&t, NULL) == -1)
	{
		return 0;
	}
	return (unsigned int)(t.tv_sec * 1000LL + t.tv_usec / 1000LL);
}

#pragma region region_ButtonState__
static std::string Button2String(int btn, bool left)
{
	std::string str;
	if (btn & BT_Trigger) str += "Trigger|";
	if (btn & BT_Grip) str += "Grip|";
	if (btn & BT_JoyStick) str += "JoyStick|";
	if (left)
	{
		if (btn & BT_Home_Menu) str += "Menu|";
		if (btn & BT_A_X) str += "X|";
		if (btn & BT_B_Y) str += "Y|";
	}
	else
	{
		if (btn & BT_Home_Menu) str += "Home|";
		if (btn & BT_A_X) str += "A|";
		if (btn & BT_B_Y) str += "B|";
	}
	return str;
}

enum class EButtonState
{
	BS_Press,	//Up -> Down
	BS_Release,	//Down -> Up
	BS_Down,	//hold after Press
	BS_Up 		//default state
};
enum EPressButtonType
{
	//[0.0f, 1.0f]
	PB_Trigger = 0,
	PB_Grip,
	//bool
	PB_A_X,
	PB_B_Y,
	PB_Home_Menu,
	PB_COUNT
};
EButtonState ctrlBtnState[PB_COUNT][CI_COUNT];
static void SetButtonState(EButtonState& state, bool down)
{
	if (down)
		state = (state == EButtonState::BS_Up) ? EButtonState::BS_Press : EButtonState::BS_Down;
	else
		state = (state == EButtonState::BS_Down) ? EButtonState::BS_Release : EButtonState::BS_Up;
}
static void UpdateButtonState(const qiyu_ControllerData* leftData, const qiyu_ControllerData* rightData)
{
	SetButtonState(ctrlBtnState[PB_Trigger][CI_Left], (leftData->triggerForce > 0.5f));//NOTE! here use 0.5f as threshold. use any value as you wish.
	SetButtonState(ctrlBtnState[PB_Grip][CI_Left], (leftData->gripForce > 0.5f));//NOTE! here use 0.5f as threshold. use any value as you wish.
	SetButtonState(ctrlBtnState[PB_A_X][CI_Left], (leftData->button & BT_A_X));
	SetButtonState(ctrlBtnState[PB_B_Y][CI_Left], (leftData->button & BT_B_Y));
	SetButtonState(ctrlBtnState[PB_Home_Menu][CI_Left], (leftData->button & BT_Home_Menu));
	//
	SetButtonState(ctrlBtnState[PB_Trigger][CI_Right], (rightData->triggerForce > 0.5f));//NOTE! here use 0.5f as threshold. use any value as you wish.
	SetButtonState(ctrlBtnState[PB_Grip][CI_Right], (rightData->gripForce > 0.5f));//NOTE! here use 0.5f as threshold. use any value as you wish.
	SetButtonState(ctrlBtnState[PB_A_X][CI_Right], (rightData->button & BT_A_X));
	SetButtonState(ctrlBtnState[PB_B_Y][CI_Right], (rightData->button & BT_B_Y));
	SetButtonState(ctrlBtnState[PB_Home_Menu][CI_Right], (rightData->button & BT_Home_Menu));
}
static EButtonState GetButtonState(qiyu_ControllerIndex idx, EPressButtonType type)
{
	return ctrlBtnState[type][idx];
}
static const char* LogButtonState(qiyu_ControllerIndex idx, EPressButtonType type)
{
	switch (ctrlBtnState[type][idx])
	{
	case EButtonState::BS_Press:
		return "Press";
		break;
	case EButtonState::BS_Release:
		return "Release";
		break;
	case EButtonState::BS_Down:
		return "Down";
		break;
	case EButtonState::BS_Up:
		return "Up";
		break;
	default:
		break;
	}
}
#pragma endregion//region_ButtonState__



#define TEST_app_id_ 		""
#define TEST_app_secret_ 	""
void Callback_PlatformAccountInfo(const PResult_AccountInfo& result)
{
	LOGI_("@@Callback_PlatformAccountInfo, result.IsSuccess:%d", result.IsSuccess());
	if (result.IsSuccess())
	{
		LOGI_("Callback_PlatformAccountInfo, result.uid= %s", result.uid);
		LOGI_("Callback_PlatformAccountInfo, result.name= %s", result.name);
		LOGI_("Callback_PlatformAccountInfo, result.icon= %s", result.icon);
	}
}
void Callback_PlatformDeepLink(const PResult_DeepLink& result)
{
	LOGI_("@@Callback_PlatformDeepLink, result.IsSuccess:%d", result.IsSuccess());
	if (result.IsSuccess())
	{
		LOGI_("Callback_PlatformDeepLink, result.appID= %s", result.appID);
		LOGI_("Callback_PlatformDeepLink, result.key= %s", result.key);
		LOGI_("Callback_PlatformDeepLink, result.value= %s", result.value);
	}
}

static void test__qiyu_Platform_IsAccountLogin()
{
	bool bLogin = qiyu_Platform_IsAccountLogin();
	LOGI_("qiyu_Platform_IsAccountLogin = %d", bLogin);
}
static void test__qiyu_Platform_GetAccountInfo()
{
	qiyu_Platform_GetAccountInfo(Callback_PlatformAccountInfo);
}
static void test__qiyu_Platform_GetDeepLink()
{
	qiyu_Platform_GetDeepLink(Callback_PlatformDeepLink);
}
void Callback_PlatformInit(const PResult_Init& result)
{
	LOGI_("@@Callback_PlatformInit, result.IsSuccess(%d), result.code(%s)", result.IsSuccess(), result.code);
	if (result.IsSuccess())
	{
		//s_PlatformInit_Success = true;
	}
}
static void test__qiyu_Platform_Init()
{
	LOGI_("qiyu_Platform_Init, app_id(%s), app_secret(%s)", TEST_app_id_, TEST_app_secret_);
	qiyu_Platform_Init(TEST_app_id_, TEST_app_secret_, Callback_PlatformInit);//get app page from developer backend.
}

static void test__init_QiyuPrefs()
{
	qiyu_Prefs_Init();
	LOGI_("qiyu_Prefs_Init");
}
static void test__set_QiyuPrefs()
{
	qiyu_Prefs_SetInt("Int", 111);
	LOGI_("qiyu_Prefs_SetInt: 111");
	qiyu_Prefs_SetFloat("Float", 0.111f);
	LOGI_("qiyu_Prefs_SetFloat: 0.111f");
	qiyu_Prefs_SetString("String", "aaaaa");
	LOGI_("qiyu_Prefs_SetString: aaaaa");
}
static void test__get_QiyuPrefs()
{
	LOGI_("~~qiyu_Prefs_GetInt: %d", qiyu_Prefs_GetInt("Int", 0));
	LOGI_("~~qiyu_Prefs_GetFloat: %f", qiyu_Prefs_GetFloat("Float", 0.0f));
	LOGI_("~~qiyu_Prefs_GetString: %s", qiyu_Prefs_GetString("String", "defaultVal"));
}
static void test__hasKey_QiyuPrefs()
{
	LOGI_("~~qiyu_Prefs_HasKey(Int): %d", qiyu_Prefs_HasKey("Int"));
	LOGI_("~~qiyu_Prefs_HasKey(Float): %d", qiyu_Prefs_HasKey("Float"));
	LOGI_("~~qiyu_Prefs_HasKey(String): %d", qiyu_Prefs_HasKey("String"));
}
static void test__delKey_QiyuPrefs()
{
	qiyu_Prefs_DeleteKey("Int");
	LOGI_("qiyu_Prefs_DeleteKey(Int)");
	qiyu_Prefs_DeleteKey("Float");
	LOGI_("qiyu_Prefs_DeleteKey(Float)");
	qiyu_Prefs_DeleteKey("String");
	LOGI_("qiyu_Prefs_DeleteKey(String)");
}
static void test__delALL_QiyuPrefs()
{
	qiyu_Prefs_DeleteAll();
	LOGI_("qiyu_Prefs_DeleteAll");
}
static void test__save_QiyuPrefs()
{
	qiyu_Prefs_Save();
	LOGI_("qiyu_Prefs_Save");
}

static void QYApp_Update(ControllerState& ctrlState)
{
	unsigned int curTime = GetCurTime();//in milliseconds
	if (g_uLastTime == 0)
	{
		g_uLastTime = curTime;
	}

	float deltaTime = (float)(curTime - g_uLastTime) / 1000.0f;//in seconds
	qiyu_Update(deltaTime);

	s_fRotateAmount = deltaTime * (360.0f / 5.0f/*RotateSpeed*/);


	//bool playSound1 = false;
	//bool playSound2 = false;
	//bool playSound3 = false;
	ctrlState.bControllerIsInit = qiyu_IsControllerInit();
	LOGV_("$TEST$ IsControllerInit, bControllerIsInit(%d)", ctrlState.bControllerIsInit);
	if (ctrlState.bControllerIsInit)
	{
		qiyu_ControllerData* left = &ctrlState.qyCtrlData[CI_Left];
		qiyu_ControllerData* right = &ctrlState.qyCtrlData[CI_Right];
		qiyu_GetControllerData(left, right);
		UpdateButtonState(left, right);

		LOGV_("$TEST$ (Left)GetControllerData, isConnect(%d), button(%s), buttonTouch(%s), batteryLevel(%d), triggerForce(%f), gripForce(%f), isShow(%d), joyStickPos(%f, %f), position(%f, %f), rotation(%f, %f, %f, %f), velocity(%f, %f, %f), acceleration(%f, %f, %f), angVelocity(%f, %f, %f), angAcceleration(%f, %f, %f)", left->isConnect, Button2String(left->button, true).c_str(), Button2String(left->buttonTouch, true).c_str(), left->batteryLevel, left->triggerForce, left->gripForce, left->isShow, left->joyStickPos.x, left->joyStickPos.y, left->position.x, left->position.y, left->rotation.x, left->rotation.y, left->rotation.z, left->rotation.w, left->velocity.x, left->velocity.y, left->velocity.z, left->acceleration.x, left->acceleration.y, left->acceleration.z, left->angVelocity.x, left->angVelocity.y, left->angVelocity.z, left->angAcceleration.x, left->angAcceleration.y, left->angAcceleration.z);
		LOGV_("$TEST$ (Right)GetControllerData, isConnect(%d), button(%s), buttonTouch(%s), batteryLevel(%d), triggerForce(%f), gripForce(%f), isShow(%d), joyStickPos(%f, %f), position(%f, %f), rotation(%f, %f, %f, %f), velocity(%f, %f, %f), acceleration(%f, %f, %f), angVelocity(%f, %f, %f), angAcceleration(%f, %f, %f)", right->isConnect, Button2String(right->button, false).c_str(), Button2String(right->buttonTouch, false).c_str(), right->batteryLevel, right->triggerForce, right->gripForce, right->isShow, right->joyStickPos.x, right->joyStickPos.y, right->position.x, right->position.y, right->rotation.x, right->rotation.y, right->rotation.z, right->rotation.w, right->velocity.x, right->velocity.y, right->velocity.z, right->acceleration.x, right->acceleration.y, right->acceleration.z, right->angVelocity.x, right->angVelocity.y, right->angVelocity.z, right->angAcceleration.x, right->angAcceleration.y, right->angAcceleration.z);
		//
		LOGV_("$TEST$ (Left)ButtonState, Trigger(%s), Grip(%s), X(%s), Y(%s)", LogButtonState(CI_Left, PB_Trigger), LogButtonState(CI_Left, PB_Grip), LogButtonState(CI_Left, PB_A_X), LogButtonState(CI_Left, PB_B_Y));
		LOGV_("$TEST$ (Right)ButtonState, Trigger(%s), Grip(%s), A(%s), B(%s)", LogButtonState(CI_Right, PB_Trigger), LogButtonState(CI_Right, PB_Grip), LogButtonState(CI_Right, PB_A_X), LogButtonState(CI_Right, PB_B_Y));

		bool bTriggerR_press = (GetButtonState(CI_Right, PB_Trigger) == EButtonState::BS_Press);
		bool bGripR_press = (GetButtonState(CI_Right, PB_Grip) == EButtonState::BS_Press);
		bool bA_press = (GetButtonState(CI_Right, PB_A_X) == EButtonState::BS_Press);
		bool bB_press = (GetButtonState(CI_Right, PB_B_Y) == EButtonState::BS_Press);
		bool bTouchA = ctrlState.qyCtrlData[CI_Right].buttonTouch & BT_A_X;
		bool bTouchB = ctrlState.qyCtrlData[CI_Right].buttonTouch & BT_B_Y;
		//
		bool bTriggerL_press = (GetButtonState(CI_Left, PB_Trigger) == EButtonState::BS_Press);
		bool bGripL_press = (GetButtonState(CI_Left, PB_Grip) == EButtonState::BS_Press);
		bool bGripL_up = (GetButtonState(CI_Left, PB_Grip) == EButtonState::BS_Up);
		bool bX_press = (GetButtonState(CI_Left, PB_A_X) == EButtonState::BS_Press);
		bool bY_press = (GetButtonState(CI_Left, PB_B_Y) == EButtonState::BS_Press);
		bool bTouchX = ctrlState.qyCtrlData[CI_Left].buttonTouch & BT_A_X;
		bool bTouchY = ctrlState.qyCtrlData[CI_Left].buttonTouch & BT_B_Y;

		if (bTriggerR_press)//<TriggerR>
		{
			s_TestGroup = (s_TestGroup+1) % TG_COUNT;
			LOGE_("********* Switch to TestGroup( %s )", s_szTestGroup[s_TestGroup]);
		}

		switch (s_TestGroup)
		{
		case TG_Default:
		{
			//{//right
			// test ControllerVibration
			if (bGripR_press)//<GripR>
			{
				qiyu_StartControllerVibration(qiyu_ControllerMask::CM_Right, 0.5f, 200.0f);
				LOGI_("$TEST$ (Right-Grip)qiyu_StartControllerVibration, amplitude(%f), duration(%f)", 0.5f, 1.5f);

				//playSound2 = true;
				//LOGI_("$TEST$ test FMOD, playSound2.");
			}
			// test TrackingOriginMode
			if (bA_press)//<Press_A>
			{
				qiyu_TrackingOriginMode curMode = qiyu_GetTrackingOriginMode();
				if (curMode == qiyu_TrackingOriginMode::TM_Device)
				{
					qiyu_SetTrackingOriginMode(qiyu_TrackingOriginMode::TM_Ground);
					LOGI_("$TEST$ qiyu_SetTrackingOriginMode, Ground mode.");
				}
				else
				{
					assert(curMode == qiyu_TrackingOriginMode::TM_Ground);
					qiyu_SetTrackingOriginMode(qiyu_TrackingOriginMode::TM_Device);
					LOGI_("$TEST$ qiyu_SetTrackingOriginMode, Device mode.");
				}

				//playSound3 = true;
				//LOGI_("$TEST$ test FMOD, playSound3.");
			}
			if (bB_press)//<Press_B>
			{
				QYApp_testFoveation();
			}
			//}
			//{//left
			if (bTriggerL_press)//<TriggerL>
			{
				qiyu_StartControllerVibration(qiyu_ControllerMask::CM_Left, 1.0f, 4.0f);
				LOGI_("$TEST$ (Left-Trigger)qiyu_StartControllerVibration, amplitude(%f), duration(%f)", 1.0f, 4.0f);

				//playSound1 = true;
				//LOGI_("$TEST$ test FMOD, playSound1.");
			}
			// test Rotate the teapots or not rotate
			s_bRotateTeapots = !bGripL_up;//<GripL>
			if (bGripL_press)//<GripL>
			{
				qiyu_StopControllerVibration(qiyu_ControllerMask::CM_Left);
				LOGI_("$TEST$ (Left-Grip)qiyu_StopControllerVibration");
			}
			//
			if (bX_press)//<Press_X>
			{
				QYApp_switchEyeTargetSize();
			}
			//
			if (bY_press)//<Press_Y>
			{
				QYApp_testMultiView();
			}
			//}
		}
		break;

		case TG_QiyuPlatform:
		{
			if (bA_press)//<Press_A>
			{
				LOGI_("$TEST$ test__qiyu_Platform_Init BEGIN...");
				test__qiyu_Platform_Init();
				LOGI_("$TEST$ test__qiyu_Platform_Init END...");				
			}
			if (bTouchX)//<Touch_X>
			{
				LOGI_("$TEST$ test__qiyu_Platform_IsAccountLogin BEGIN...");
				test__qiyu_Platform_IsAccountLogin();
				LOGI_("$TEST$ test__qiyu_Platform_IsAccountLogin END...");
			}
			if (bB_press)//<Press_B>
			{
				LOGI_("$TEST$ test__qiyu_Platform_GetAccountInfo BEGIN...");
				test__qiyu_Platform_GetAccountInfo();
				LOGI_("$TEST$ test__qiyu_Platform_GetAccountInfo END...");
			}
			if (bY_press)//<Press_Y>
			{
			}
			if (bGripL_press)//<GripL>
			{
				LOGI_("$TEST$ test__qiyu_Platform_GetDeepLink BEGIN...");
				test__qiyu_Platform_GetDeepLink();
				LOGI_("$TEST$ test__qiyu_Platform_GetDeepLink END...");
			}
		}
		break;

		case TG_QiyuPrefs:
		{
			if (bA_press)//<Press_A>
			{			
				LOGI_("$TEST$ test__init_QiyuPrefs BEGIN...");
				test__init_QiyuPrefs();
				LOGI_("$TEST$ test__init_QiyuPrefs END...");
			}
			if (bB_press)//<Press_B>
			{
				LOGI_("$TEST$ test__set_QiyuPrefs BEGIN...");
				test__set_QiyuPrefs();
				LOGI_("$TEST$ test__set_QiyuPrefs END...");
			}
			if (bTouchB)//<Touch_B>
			{
				LOGI_("$TEST$ test__get_QiyuPrefs BEGIN...");
				test__get_QiyuPrefs();
				LOGI_("$TEST$ test__get_QiyuPrefs END...");
			}
			if (bX_press)//<Press_X>
			{
				LOGI_("$TEST$ test__hasKey_QiyuPrefs BEGIN...");
				test__hasKey_QiyuPrefs();
				LOGI_("$TEST$ test__hasKey_QiyuPrefs END...");
			}
			if (bY_press)//<Press_Y>
			{
				LOGI_("$TEST$ test__delKey_QiyuPrefs BEGIN...");
				test__delKey_QiyuPrefs();
				LOGI_("$TEST$ test__delKey_QiyuPrefs END...");
			}
			if (bTriggerL_press)//<TriggerL>
			{
				LOGI_("$TEST$ test__delALL_QiyuPrefs BEGIN...");
				test__delALL_QiyuPrefs();
				LOGI_("$TEST$ test__delALL_QiyuPrefs END...");
			}
			if (bGripL_press)//<GripL>
			{
				LOGI_("$TEST$ test__save_QiyuPrefs BEGIN...");
				test__save_QiyuPrefs();
				LOGI_("$TEST$ test__save_QiyuPrefs END...");
			}
		}
		break;

		case TG_Boundary:
		{
			if (bA_press)//<Press_A>
			{
				enable_OUTPUT_VERBOSE_ = !enable_OUTPUT_VERBOSE_;
				LOGI_("$TEST$ enable_OUTPUT_VERBOSE_(%d)", enable_OUTPUT_VERBOSE_);
			}
			if (bB_press)//<Press_B>
			{
				if (qiyu_GetBoundaryGeometry(g_pointsBoundaryGeometry))
				{
					int points_count = g_pointsBoundaryGeometry.size();
					for (int i = 0; i < points_count; i++)
					{
						const qiyu_Vector3& v = g_pointsBoundaryGeometry.at(i);
						LOGI_("$TEST$ qiyu_GetBoundaryGeometry, succeed to get pointsData. point_index(%d) / points_count(%d), point_data(%f, %f, %f)", i + 1, points_count, v.x, v.y, v.z);
					}
				}
				else
				{
					LOGI_("$TEST$ fail to qiyu_GetBoundaryGeometry");
				}
			}
			if (bTriggerL_press)//<TriggerL>
			{
				qiyu_Vector3 v = qiyu_GetBoundaryDimensions();
				LOGI_("$TEST$ qiyu_GetBoundaryDimensions, xyz(%f, %f, %f)", v.x, v.y, v.z);
			}
			if (bX_press)//<Press_X>
			{
				bool bVisible = qiyu_IsBoundaryVisible();
				LOGI_("$TEST$ qiyu_IsBoundaryVisible, bVisible(%d)", bVisible);
			}
			if (bY_press)//<Press_Y>
			{
				bool bVisible = qiyu_IsBoundaryBelowHeadVisible();
				LOGI_("$TEST$ qiyu_IsBoundaryBelowHeadVisible, bVisible(%d)", bVisible);
			}
		}
		break;

		default:
			assert(0 && "ILLEGAL s_TestGroup");
			break;
		}
	}

	//if (g_pQYFmod && g_pQYFmod->IsInitialized())
	//{
	//	g_pQYFmod->Update(playSound1, playSound2, playSound3);
	//}

	g_uLastTime = curTime;
}

static void QYApp_UpdateAndRender(bool bMultiView, bool bIsCustomScaleRT)
{
	ControllerState ctrlState;
	QYApp_Update(ctrlState);

	QYApp_PreRender();
	QYApp_Render(bMultiView, bIsCustomScaleRT, ctrlState);
	QYApp_PostRender();
}

static bool QYApp_Release(QYEgl* egl)
{
	if (!g_isInited)//
	{
		//LOGW_("@@QYApp_Release, alread release");
		return false;
	}

	if (!qiyu_Release())
	{
		LOGE_("@@QYApp_Release, fail to qiyu_Release");
		// return false;//FIXME!
	}

	//if (g_pQYFmod)
	//{
	//	g_pQYFmod->Release();
	//	delete g_pQYFmod;
	//	g_pQYFmod = nullptr;
	//}

	if (g_pHandleRayShader)//
	{
		delete g_pHandleRayShader;
		g_pHandleRayShader = nullptr;
	}
	if (g_pHandleRayShader_MultiView)//
	{
		delete g_pHandleRayShader_MultiView;
		g_pHandleRayShader_MultiView = nullptr;
	}
	if (g_pHandleRayLR)
	{
		delete g_pHandleRayLR;
		g_pHandleRayLR = nullptr;
	}

	if (g_pHandleShader)
	{
		delete g_pHandleShader;
		g_pHandleShader = nullptr;
	}
	if (g_pHandleShader_MultiView)
	{
		delete g_pHandleShader_MultiView;
		g_pHandleShader_MultiView = nullptr;
	}
	if (g_pHandleL)
	{
		delete g_pHandleL;
		g_pHandleL = nullptr;
	}
	if (g_pHandleR)
	{
		delete g_pHandleR;
		g_pHandleR = nullptr;
	}

	if (g_pTeapotShader)
	{
		delete g_pTeapotShader;
		g_pTeapotShader = nullptr;
	}
	if (g_pTeapotShader_MultiView)
	{
		delete g_pTeapotShader_MultiView;
		g_pTeapotShader_MultiView = nullptr;
	}
	if (g_pTeapot)
	{
		delete g_pTeapot;
		g_pTeapot = nullptr;
	}

	QYApp_ReleaseEyeBuffers();//
	QYApp_DestroyContext(egl);

	g_isInited = false;
	return true;
}

static void QYApp_PauseVR()
{
	g_isPaused = true;//FIXME!
	if (g_isStartedVR)
	{
		if (qiyu_EndVR())
		{
			g_isStartedVR = false;
			g_uLastTime = 0;//
		}
		else
		{
			LOGE_("@@QYApp_PauseVR, fail to qiyu_EndVR");
		}
	}
}

/**
 * Process the next main command.
 */
static void CommandCallback(struct android_app* app, int32_t cmd)
{
	QYApp* qyApp = (QYApp*)app->userData;
	switch (cmd)
	{
		// There is no APP_CMD_CREATE. The ANativeActivity creates the
		// application thread from onCreate(). The application thread
		// then calls android_main().
		// Command from main thread: the app's activity has been started.
	case APP_CMD_START:
	{
		LOGI_("@@CommandCallback, cmdID: APP_CMD_START");
		break;
	}
	// Command from main thread: the app's activity has been resumed.
	case APP_CMD_RESUME:
	{
		LOGI_("@@CommandCallback, cmdID: APP_CMD_RESUME");
		g_isPaused = false;

		break;
	}
	// Command from main thread: the app's activity has been paused.
	case APP_CMD_PAUSE:
	{
		LOGI_("@@CommandCallback, cmdID: APP_CMD_PAUSE");

		break;
	}
	// Command from main thread: the app's activity has been stopped.
	case APP_CMD_STOP:
	{
		LOGI_("@@CommandCallback, cmdID: APP_CMD_STOP");
		QYApp_PauseVR();//!
		break;
	}
	// Command from main thread: the app's activity is being destroyed,
	// and waiting for the app thread to clean up and exit before proceeding.
	case APP_CMD_DESTROY:
	{
		LOGI_("@@CommandCallback, cmdID: APP_CMD_DESTROY");
		QYApp_Release(&qyApp->qyEgl);//!
		break;
	}
	// Command from main thread: a new ANativeWindow is ready for use.  Upon
	// receiving this command, android_app->window will contain the new window
	// surface.surfaceCreated
	case APP_CMD_INIT_WINDOW:
	{
		LOGI_("@@CommandCallback, cmdID: APP_CMD_INIT_WINDOW");
		break;
	}
	// Command from main thread: the existing ANativeWindow needs to be
	// terminated.  Upon receiving this command, android_app->window still
	// contains the existing window; after calling android_app_exec_cmd
	// it will be set to NULL.surfaceDestroyed
	case APP_CMD_TERM_WINDOW:
	{
		LOGI_("@@CommandCallback, cmdID: APP_CMD_TERM_WINDOW");
		
		break;
	}


	// Command from main thread: the app should generate a new saved state
	// for itself, to restore from later if needed.  If you have saved state,
	// allocate it with malloc and place it in android_app.savedState with
	// the size in android_app.savedStateSize.  The will be freed for you
	// later.
	case APP_CMD_SAVE_STATE:
		LOGI_("@@CommandCallback, cmdID: APP_CMD_SAVE_STATE");
		break;

		// Command from main thread: the AInputQueue has changed.  Upon processing
		// this command, android_app->inputQueue will be updated to the new queue
		// (or NULL).
	case APP_CMD_INPUT_CHANGED:
		LOGI_("@@CommandCallback, cmdID: APP_CMD_INPUT_CHANGED");
		break;

		// Command from main thread: the current ANativeWindow has been resized.
		// Please redraw with its new size.
	case APP_CMD_WINDOW_RESIZED:
		LOGI_("@@CommandCallback, cmdID: APP_CMD_WINDOW_RESIZED");
		break;

		// Command from main thread: the system needs that the current ANativeWindow
		// be redrawn.  You should redraw the window before handing this to
		// android_app_exec_cmd() in order to avoid transient drawing glitches.
	case APP_CMD_WINDOW_REDRAW_NEEDED:
		LOGI_("@@CommandCallback, cmdID: APP_CMD_WINDOW_REDRAW_NEEDED");
		break;

		// Command from main thread: the content area of the window has changed,
		// such as from the soft input window being shown or hidden.  You can
		// find the new content rect in android_app::contentRect.
	case APP_CMD_CONTENT_RECT_CHANGED:
		LOGI_("@@CommandCallback, cmdID: APP_CMD_CONTENT_RECT_CHANGED");
		break;

		// Command from main thread: the app's activity window has gained
		// input focus.
	case APP_CMD_GAINED_FOCUS:
		LOGI_("@@CommandCallback, cmdID: APP_CMD_GAINED_FOCUS");
		break;

		// Command from main thread: the app's activity window has lost
		// input focus.
	case APP_CMD_LOST_FOCUS:
		LOGI_("@@CommandCallback, cmdID: APP_CMD_LOST_FOCUS");
		break;

		// Command from main thread: the current device configuration has changed.
	case APP_CMD_CONFIG_CHANGED:
		LOGI_("@@CommandCallback, cmdID: APP_CMD_CONFIG_CHANGED");
		break;

		// Command from main thread: the system is running low on memory.
		// Try to reduce your memory use.
	case APP_CMD_LOW_MEMORY:
		LOGI_("@@CommandCallback, cmdID: APP_CMD_LOW_MEMORY");
		break;


	default:
		LOGI_("@@CommandCallback, cmdID: Unknown APP_CMD_: %d", cmd);
		break;
	}
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* app)
{
	LOGI_("@@android_main, START...");
	
	// Init helper functions
	ndk_helper::JNIHelper::GetInstance()->Init(app->activity, HELPER_CLASS_NAME, "qiyuapi");
	LOGV("@@android_main, QiyuApi loadLibrary, qiyuapi, done");

	QYApp qyApp;
	qyApp.pANativeActivity = app->activity;
	qyApp.szExternalPath = (char*)app->activity->externalDataPath;

	if (!QYApp_Init(&qyApp))
	{
		LOGE_("@@android_main, fail to QYApp_Init");
		QYApp_Release(&qyApp.qyEgl);
		return;
	}
	LOGI_("@@android_main, succeed to QYApp_Init");

	app->userData = &qyApp;
	app->onAppCmd = CommandCallback;

	while (1)
	{
		int iIdent, iEvents;
		struct android_poll_source* pAndroidPollSource;

		while ((iIdent = ALooper_pollAll(0, nullptr, &iEvents, (void**)&pAndroidPollSource)) >= 0)
		{
			if (pAndroidPollSource != nullptr)
			{
				pAndroidPollSource->process(app, pAndroidPollSource);
			}
		}

		g_isPaused = false;//FIXME!
		if ((g_isPaused == false) && (g_isStartedVR == false))
		{
			assert(app);
			if (app && app->window)
			{
				g_isStartedVR = qiyu_StartVR(app->window, PL_Medium, PL_Medium);
			}
		}

		if (g_isInited && g_isStartedVR)
		{
			QYApp_UpdateAndRender(s_bIsMultiView, s_bIsCustomScaleRT);
		}
	}

	QYApp_Release(&qyApp.qyEgl);
	LOGI_("@@android_main, END...");
}
