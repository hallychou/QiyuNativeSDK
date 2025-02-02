/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 //--------------------------------------------------------------------------------
 // Include files
 //--------------------------------------------------------------------------------
#include <jni.h>
#include <errno.h>

#include <vector>
#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/native_window_jni.h>

#include "MoreTeapotsRenderer.h"

#include "QYRenderTarget.h"

//-------------------------------------------------------------------------
// Preprocessor
//-------------------------------------------------------------------------
#define HELPER_CLASS_NAME \
  "com/sample/helper/NDKHelper"  // Class name of helper function
//-------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------
const int32_t NUM_TEAPOTS_X = 4;//8;
const int32_t NUM_TEAPOTS_Y = 3;//8;
const int32_t NUM_TEAPOTS_Z = 2;//8;

//!!QiyuApi
#define NUM_EYE_BUFFERS_     4 //FIXME! //TODO!
//-------------------------------------------------------------------------
// Shared state for our app.
//-------------------------------------------------------------------------
struct android_app;
class Engine {

	//!!QiyuApi
	struct QYEyeBuffer
	{
		QYRenderTarget eyeTarget[EYE_COUNT];
	};
	QYEyeBuffer		eyeBuffers[NUM_EYE_BUFFERS_];
	int				eyeBufferIndex = 0;
	//
	void AllocateEyeBuffers()
	{
		LOGV("@@Engine::AllocateEyeBuffers, BEGIN...");
		qiyu_DeviceInfo deviceInfo = qiyu_GetDeviceInfo();
		//Default to separate render targets per eye
		LOGV("@@Engine::AllocateEyeBuffers, deviceInfo.iEyeTargetWidth(%d), deviceInfo.iEyeTargetHeight(%d)", deviceInfo.iEyeTargetWidth, deviceInfo.iEyeTargetHeight);
		for (int i = 0; i < NUM_EYE_BUFFERS_; i++)
		{
			for (int k = 0; k < EYE_COUNT; k++)
			{
				eyeBuffers[i].eyeTarget[k].Init(false, 0, false, deviceInfo.iEyeTargetWidth, deviceInfo.iEyeTargetHeight, 1, GL_RGBA8, true, false);
			}
		}
		LOGV("@@Engine::AllocateEyeBuffers, END...");
	}
	void FreeEyeBuffers()//void Shutdown()
	{
		LOGV("@@Engine::FreeEyeBuffers, BEGIN...");
		for (int i = 0; i < NUM_EYE_BUFFERS_; i++)
		{
			for (int k = 0; k < EYE_COUNT; k++)
			{
				eyeBuffers[i].eyeTarget[k].Release();
			}
		}
		LOGV("@@Engine::FreeEyeBuffers, END...");
	}


	MoreTeapotsRenderer renderer_;

	ndk_helper::GLContext* gl_context_;

	bool initialized_resources_;
	bool has_focus_;

	ndk_helper::DoubletapDetector doubletap_detector_;
	ndk_helper::PinchDetector pinch_detector_;
	ndk_helper::DragDetector drag_detector_;
	ndk_helper::PerfMonitor monitor_;

	//ndk_helper::TapCamera tap_camera_;//!!

	android_app* app_;

	ASensorManager* sensor_manager_;
	const ASensor* accelerometer_sensor_;
	ASensorEventQueue* sensor_event_queue_;

	void UpdateFPS(float fps);
	void ShowUI();
	void TransformPosition(ndk_helper::Vec2& vec);

public:
	static void HandleCmd(struct android_app* app, int32_t cmd);
	static int32_t HandleInput(android_app* app, AInputEvent* event);

	Engine();
	~Engine();
	void SetState(android_app* app);
	int InitDisplay(android_app* app);
	void LoadResources();
	void UnloadResources();
	void DrawFrame();
	void TermDisplay();
	void TrimMemory();
	bool IsReady();

	void UpdatePosition(AInputEvent* event, int32_t index, float& x, float& y);

	void InitSensors();
	void ProcessSensors(int32_t id);
	void SuspendSensors();
	void ResumeSensors();
};

//-------------------------------------------------------------------------
// Ctor
//-------------------------------------------------------------------------
Engine::Engine()
	: initialized_resources_(false),
	has_focus_(false),
	app_(NULL),
	sensor_manager_(NULL),
	accelerometer_sensor_(NULL),
	sensor_event_queue_(NULL) {
	gl_context_ = ndk_helper::GLContext::GetInstance();
}

//-------------------------------------------------------------------------
// Dtor
//-------------------------------------------------------------------------
Engine::~Engine() {}

/**
 * Load resources
 */
void Engine::LoadResources() {
	LOGV("@@Engine::LoadResources, BEGIN...");
	renderer_.Init(NUM_TEAPOTS_X, NUM_TEAPOTS_Y, NUM_TEAPOTS_Z);
	//renderer_.Bind(&tap_camera_);//!!
	LOGV("@@Engine::LoadResources, END...");
}

/**
 * Unload resources
 */
void Engine::UnloadResources() {
	LOGV("@@Engine::UnloadResources, BEGIN...");
	renderer_.Unload();
	LOGV("@@Engine::UnloadResources, END...");
}

/**
 * Initialize an EGL context for the current display.
 */
int Engine::InitDisplay(android_app *app)
{
	LOGV("@@Engine::InitDisplay, BEGIN...");
	if (!initialized_resources_)
	{
		LOGV("@@Engine::InitDisplay, if (!initialized_resources_)");
		gl_context_->Init(app_->window);
		LoadResources();
		initialized_resources_ = true;
	}
	else if (app->window != gl_context_->GetANativeWindow())
	{
		LOGV("@@Engine::InitDisplay, else if(app->window != gl_context_->GetANativeWindow())");
		// Re-initialize ANativeWindow.
		// On some devices, ANativeWindow is re-created when the app is resumed
		assert(gl_context_->GetANativeWindow());
		UnloadResources();
		gl_context_->Invalidate();
		app_ = app;
		gl_context_->Init(app->window);
		LoadResources();
		initialized_resources_ = true;
	}
	else
	{
		LOGV("@@Engine::InitDisplay, else");
		// initialize OpenGL ES and EGL
		if (EGL_SUCCESS == gl_context_->Resume(app_->window))
		{
			UnloadResources();
			LoadResources();
		}
		else
		{
			assert(0);
		}
	}

	ShowUI();

	// Initialize GL state.
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Note that screen size might have been changed
	glViewport(0, 0, gl_context_->GetScreenWidth(), gl_context_->GetScreenHeight());//FIXME!
	LOGV("@@Engine::InitDisplay, gl_context_->GetScreenWidth(%d), gl_context_->GetScreenHeight(%d)", gl_context_->GetScreenWidth(), gl_context_->GetScreenHeight());
	//renderer_.UpdateViewport();//!!

	//tap_camera_.SetFlip(1.f, -1.f, -1.f);//!!
	//tap_camera_.SetPinchTransformFactor(10.f, 10.f, 8.f);//!!


	AllocateEyeBuffers();
	//FIXME! //TODO!
	////Could be called again, if native window changes. End previous start first.
	//qiyu_EndVR//EndVR(pApp);
	if (!qiyu_StartVR(app->window, PL_Medium, PL_Medium))
	{
		LOGE("@@Engine::InitDisplay, fail to qiyu_StartVR");
		return -1;//FIXME!
	}
	LOGV("@@Engine::InitDisplay, qiyu_StartVR done");


	LOGV("@@Engine::InitDisplay, END...");
	return 0;
}

static void CreateLayout_(float centerX, float centerY, float radiusX, float radiusY, qiyu_RenderLayer_ScreenPosUV* pLayout)//!
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
/**
 * Just the current frame in the display.
 */
void Engine::DrawFrame()
{
	LOGV("@@Engine::DrawFrame, BEGIN...");
	float fps;
	float tickSecond;
	if (monitor_.Update(fps, &tickSecond))//if (monitor_.Update(fps))
	{
		UpdateFPS(fps);
	}
	qiyu_Update(tickSecond);
	//double dTime = monitor_.GetCurrentTime();
	renderer_.Update();//renderer_.Update(dTime);


	//FIXME! //TODO!
	bool bMultiView = false;//TEST!
	for (int e = 0; e < EYE_COUNT; ++e)
	{
		qiyu_Eye eyeType = qiyu_Eye(e);
		//if (!qiyu_StartEye(bMultiView, eyeType, (bMultiView ? TT_TextureArray : TT_Texture)))//FIXME! //TODO!
		//{
		//  LOGE("@@Engine::DrawFrame, fail to qiyu_StartEye");
		//}
		//LOGV("@@Engine::DrawFrame, qiyu_StartEye done");
		//
		eyeBuffers[eyeBufferIndex].eyeTarget[eyeType].Bind();
		// Just fill the screen with a color.
		glClearColor(0.5f, 0.5f, 0.5f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//
		if (!qiyu_StartEye(bMultiView, eyeType, (bMultiView ? TT_TextureArray : TT_Texture)))
		{
			LOGE("@@Engine::DrawFrame, fail to qiyu_StartEye");
		}
		LOGV("@@Engine::DrawFrame, qiyu_StartEye done");


		renderer_.Render(eyeType);//renderer_.Render();


		if (!qiyu_EndEye(bMultiView, eyeType, (bMultiView ? TT_TextureArray : TT_Texture)))
		{
			LOGE("@@Engine::DrawFrame, fail to qiyu_EndEye");
		}
		LOGV("@@Engine::DrawFrame, qiyu_EndEye done");
		eyeBuffers[eyeBufferIndex].eyeTarget[eyeType].UnBind();
	}

	qiyu_FrameParam frameParam;
	memset(&frameParam, 0, sizeof(frameParam));//
	frameParam.minVsyncs = 1;
	frameParam.headPoseState = renderer_.m_headPoseState;
	//!
	frameParam.renderLayers[0].imageHandle = eyeBuffers[eyeBufferIndex].eyeTarget[EYE_Left].GetColorAttachment();
	frameParam.renderLayers[0].imageType = TT_Texture;//!!
	CreateLayout_(0.0f, 0.0f, 1.0f, 1.0f, &frameParam.renderLayers[0].imageCoords);//!
	frameParam.renderLayers[0].eyeMask = RL_EyeMask_Left;//
	//
	frameParam.renderLayers[1].imageHandle = eyeBuffers[eyeBufferIndex].eyeTarget[EYE_Right].GetColorAttachment();
	frameParam.renderLayers[1].imageType = TT_Texture;//!!
	CreateLayout_(0.0f, 0.0f, 1.0f, 1.0f, &frameParam.renderLayers[1].imageCoords);//!
	frameParam.renderLayers[1].eyeMask = RL_EyeMask_Right;//
	//!
	if (!qiyu_SubmitFrame(frameParam))
	{
		LOGE("@@Engine::DrawFrame, fail to qiyu_SubmitFrame");
	}
	eyeBufferIndex = (eyeBufferIndex + 1) % NUM_EYE_BUFFERS_;
	LOGV("@@Engine::DrawFrame, qiyu_SubmitFrame done");


	// Swap
	if (EGL_SUCCESS != gl_context_->Swap()) {
		UnloadResources();
		LoadResources();
	}
	LOGV("@@Engine::DrawFrame, END...");
}

/**
 * Tear down the EGL context currently associated with the display.
 */
void Engine::TermDisplay()
{
	gl_context_->Suspend();


	//FIXME! //TODO! qiyu_EndVR @APP_CMD_TERM_WINDOW or @APP_CMD_STOP(like sxr: APP_CMD_STOP->PauseVR->EndVR->sxrEndXr&//FreeEyeBuffers)
	if (!qiyu_EndVR())
	{
		LOGE("@@Engine::TermDisplay, fail to qiyu_EndVR");
	}
	LOGV("@@Engine::TermDisplay, qiyu_EndVR done");
	FreeEyeBuffers();
}

void Engine::TrimMemory() {
	LOGI("Trimming memory");
	gl_context_->Invalidate();
}
/**
 * Process the next input event.
 */
int32_t Engine::HandleInput(android_app* app, AInputEvent* event) {
	Engine* eng = (Engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		ndk_helper::GESTURE_STATE doubleTapState =
			eng->doubletap_detector_.Detect(event);
		ndk_helper::GESTURE_STATE dragState = eng->drag_detector_.Detect(event);
		ndk_helper::GESTURE_STATE pinchState = eng->pinch_detector_.Detect(event);

		// Double tap detector has a priority over other detectors
		if (doubleTapState == ndk_helper::GESTURE_STATE_ACTION) {
			// Detect double tap
			//eng->tap_camera_.Reset(true);//!!
		}
		else {
			// Handle drag state
			if (dragState & ndk_helper::GESTURE_STATE_START) {
				// Otherwise, start dragging
				ndk_helper::Vec2 v;
				eng->drag_detector_.GetPointer(v);
				eng->TransformPosition(v);
				//eng->tap_camera_.BeginDrag(v);//!!
			}
			else if (dragState & ndk_helper::GESTURE_STATE_MOVE) {
				ndk_helper::Vec2 v;
				eng->drag_detector_.GetPointer(v);
				eng->TransformPosition(v);
				//eng->tap_camera_.Drag(v);//!!
			}
			else if (dragState & ndk_helper::GESTURE_STATE_END) {
				//eng->tap_camera_.EndDrag();//!!
			}

			// Handle pinch state
			if (pinchState & ndk_helper::GESTURE_STATE_START) {
				// Start new pinch
				ndk_helper::Vec2 v1;
				ndk_helper::Vec2 v2;
				eng->pinch_detector_.GetPointers(v1, v2);
				eng->TransformPosition(v1);
				eng->TransformPosition(v2);
				//eng->tap_camera_.BeginPinch(v1, v2);//!!
			}
			else if (pinchState & ndk_helper::GESTURE_STATE_MOVE) {
				// Multi touch
				// Start new pinch
				ndk_helper::Vec2 v1;
				ndk_helper::Vec2 v2;
				eng->pinch_detector_.GetPointers(v1, v2);
				eng->TransformPosition(v1);
				eng->TransformPosition(v2);
				//eng->tap_camera_.Pinch(v1, v2);//!!
			}
		}
		return 1;
	}
	return 0;
}

/**
 * Process the next main command.
 */
void Engine::HandleCmd(struct android_app* app, int32_t cmd)
{
	Engine* eng = (Engine*)app->userData;
	switch (cmd)
	{
	case APP_CMD_SAVE_STATE:
	{
		LOGI("@@HandleCmd, APP_CMD_SAVE_STATE");
	}
	break;
	case APP_CMD_INIT_WINDOW:
	{
		LOGI("@@HandleCmd, APP_CMD_INIT_WINDOW");

		//FIXME! //TODO!
		////BEG_#MultiDisplay_PowerButton_NoScreenLock
		//sxrEndXr();//this is required for suspend/resume via the power button to work without screen-lock.  Screen-lock causes an egl error on eglCreateWindowSurface()
		////END_#MultiDisplay_PowerButton_NoScreenLock

		// The window is being shown, get it ready.
		if (app->window != NULL) {
			eng->InitDisplay(app);//qiyu_StartVR @InitDisplay end
			eng->has_focus_ = true;
			eng->DrawFrame();
		}
	}
	break;
	case APP_CMD_TERM_WINDOW:
	{
		LOGI("@@HandleCmd, APP_CMD_TERM_WINDOW");

		// The window is being hidden or closed, clean it up.
		eng->TermDisplay();//qiyu_EndVR @TermDisplay end.  //FIXME! //TODO! qiyu_EndVR @APP_CMD_TERM_WINDOW or @APP_CMD_STOP(like sxr: APP_CMD_STOP->PauseVR->EndVR->sxrEndXr&//FreeEyeBuffers)
		eng->has_focus_ = false;
	}
	break;
	case APP_CMD_STOP:
	{
		LOGI("@@HandleCmd, APP_CMD_STOP");

		//FIXME! //TODO! qiyu_EndVR @APP_CMD_TERM_WINDOW or @APP_CMD_STOP(like sxr: APP_CMD_STOP->PauseVR->EndVR->sxrEndXr&//FreeEyeBuffers)
	}
	break;
	case APP_CMD_GAINED_FOCUS:
	{
		LOGI("@@HandleCmd, APP_CMD_GAINED_FOCUS");
		eng->ResumeSensors();
		// Start animation
		eng->has_focus_ = true;
	}
	break;
	case APP_CMD_LOST_FOCUS:
	{
		LOGI("@@HandleCmd, APP_CMD_LOST_FOCUS");
		eng->SuspendSensors();
		// Also stop animating.
		eng->has_focus_ = false;
		eng->DrawFrame();
	}
	break;
	case APP_CMD_LOW_MEMORY:
	{
		LOGI("@@HandleCmd, APP_CMD_LOW_MEMORY");
		// Free up GL resources
		eng->TrimMemory();
	}
	break;


	//!!QiyuApi //FIXME! //TODO!
		// Command from main thread: the app's activity has been started.
	case APP_CMD_START:
	{
		LOGI("@@HandleCmd, APP_CMD_START");

		//// See "Multi-Window Lifecycle" https://developer.android.com/guide/topics/ui/multi-window
		//// Instead of handling APP_CMD_RESUME and APP_CMD_PAUSE,
		//// handle APP_CMD_START and APP_CMD_STOP
		//gIsPaused = false;
		//LOG_ANDROID_FRAMEWORK_STATE("%s:%i:threadid=%i, gSvrInitialized=%i, gNeedsReInitialization=%i, gContextCreated=%i, gRecreateContext=%i, gIsPaused=%i",
		//    __FILE__, __LINE__, gettid(), gSvrInitialized, gNeedsReInitialization, gContextCreated, gRecreateContext, gIsPaused);
	}
	break;
	// Command from main thread: the app's activity has been resumed.
	case APP_CMD_RESUME:
	{
		LOGI("@@HandleCmd, APP_CMD_RESUME");
	}
	break;
	// Command from main thread: the app's activity has been paused.
	case APP_CMD_PAUSE:
	{
		LOGI("@@HandleCmd, APP_CMD_PAUSE");
	}
	break;
	// Command from main thread: the app's activity is being destroyed,
	// and waiting for the app thread to clean up and exit before proceeding.
	case APP_CMD_DESTROY:
	{
		LOGI("@@HandleCmd, APP_CMD_DESTROY");

		//FIXME! //TODO! //FIXME! where call  qiyu_Release ?
		if (!qiyu_Release())//FIXME! by current system logic, kill process @home will not arrive here
		{
			LOGE("@@HandleCmd, APP_CMD_DESTROY, fail to qiyu_Release");
		}
	}
	break;
	
	default:
		LOGE("@@HandleCmd, Unknown APP_CMD_: %d", cmd);
		break;
	}
}

//-------------------------------------------------------------------------
// Sensor handlers
//-------------------------------------------------------------------------
void Engine::InitSensors() {
	sensor_manager_ = ndk_helper::AcquireASensorManagerInstance(app_);
	accelerometer_sensor_ = ASensorManager_getDefaultSensor(
		sensor_manager_, ASENSOR_TYPE_ACCELEROMETER);
	sensor_event_queue_ = ASensorManager_createEventQueue(
		sensor_manager_, app_->looper, LOOPER_ID_USER, NULL, NULL);
}

void Engine::ProcessSensors(int32_t id) {
	// If a sensor has data, process it now.
	if (id == LOOPER_ID_USER) {
		if (accelerometer_sensor_ != NULL) {
			ASensorEvent event;
			while (ASensorEventQueue_getEvents(sensor_event_queue_, &event, 1) > 0) {
			}
		}
	}
}

void Engine::ResumeSensors() {
	// When our app gains focus, we start monitoring the accelerometer.
	if (accelerometer_sensor_ != NULL) {
		ASensorEventQueue_enableSensor(sensor_event_queue_, accelerometer_sensor_);
		// We'd like to get 60 events per second (in us).
		ASensorEventQueue_setEventRate(sensor_event_queue_, accelerometer_sensor_,
			(1000L / 60) * 1000);
	}
}

void Engine::SuspendSensors() {
	// When our app loses focus, we stop monitoring the accelerometer.
	// This is to avoid consuming battery while not being used.
	if (accelerometer_sensor_ != NULL) {
		ASensorEventQueue_disableSensor(sensor_event_queue_, accelerometer_sensor_);
	}
}

//-------------------------------------------------------------------------
// Misc
//-------------------------------------------------------------------------
void Engine::SetState(android_app* state) {
	app_ = state;
	doubletap_detector_.SetConfiguration(app_->config);
	drag_detector_.SetConfiguration(app_->config);
	pinch_detector_.SetConfiguration(app_->config);
}

bool Engine::IsReady() {
	if (has_focus_) return true;

	return false;
}

void Engine::TransformPosition(ndk_helper::Vec2& vec) {
	vec = ndk_helper::Vec2(2.0f, 2.0f) * vec /
		ndk_helper::Vec2(gl_context_->GetScreenWidth(),
			gl_context_->GetScreenHeight()) -
		ndk_helper::Vec2(1.f, 1.f);
}

void Engine::ShowUI() {
	LOGV("@@Engine::ShowUI, BEGIN...");
	JNIEnv* jni;
	app_->activity->vm->AttachCurrentThread(&jni, NULL);

	// Default class retrieval
	jclass clazz = jni->GetObjectClass(app_->activity->clazz);
	jmethodID methodID = jni->GetMethodID(clazz, "showUI", "()V");
	jni->CallVoidMethod(app_->activity->clazz, methodID);

	app_->activity->vm->DetachCurrentThread();
	LOGV("@@Engine::ShowUI, END...");
	return;
}

void Engine::UpdateFPS(float fps) {
	LOGV("@@Engine::UpdateFPS, BEGIN...");
	JNIEnv* jni;
	app_->activity->vm->AttachCurrentThread(&jni, NULL);

	// Default class retrieval
	jclass clazz = jni->GetObjectClass(app_->activity->clazz);
	jmethodID methodID = jni->GetMethodID(clazz, "updateFPS", "(F)V");
	jni->CallVoidMethod(app_->activity->clazz, methodID, fps);

	app_->activity->vm->DetachCurrentThread();
	LOGV("@@Engine::UpdateFPS, END...");
	return;
}

Engine g_engine;

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(android_app* state)
{
	LOGV("@@android_main, BEGIN...");

	g_engine.SetState(state);
	LOGV("@@android_main, g_engine.SetState, done");

	// Init helper functions
	//ndk_helper::JNIHelper::GetInstance()->Init(state->activity, HELPER_CLASS_NAME);
	//!!QiyuApi
	ndk_helper::JNIHelper::GetInstance()->Init(state->activity, HELPER_CLASS_NAME, "qiyuapi");
	LOGV("@@android_main, QiyuApi loadLibrary, qiyuapi, done");

	if (!qiyu_Init(state->activity->clazz,
					state->activity->vm,
					qiyu_GraphicsApi::GA_OpenGLES,
					qiyu_TrackingOriginMode::TM_Device,
					false))
	{
		LOGE("@@android_main, fail to qiyu_Init");
		return;//!
	}
	LOGV("@@android_main, succeed to qiyu_Init");

	state->userData = &g_engine;
	state->onAppCmd = Engine::HandleCmd;
	state->onInputEvent = Engine::HandleInput;

#ifdef USE_NDK_PROFILER
	monstartup("libMoreTeapotsNativeActivity.so");
#endif

  // Prepare to monitor accelerometer
	g_engine.InitSensors();

	// loop waiting for stuff to do.
	while (1)
	{
		// Read all pending events.
		int id;
		int events;
		android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((id = ALooper_pollAll(g_engine.IsReady() ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
		{
			// Process this event.
			if (source != NULL) source->process(state, source);

			g_engine.ProcessSensors(id);

			// Check if we are exiting.
			if (state->destroyRequested != 0)
			{
				g_engine.TermDisplay();

				//FIXME! where call  qiyu_Release ?
				////if (!qiyu_Release())//FIXME!
				////{
				////    LOGE("@@android_main, fail to qiyu_Release");
				////}
				//pAppState->activity->vm->DetachCurrentThread();
				//exit(0);    // Need to exit or some stuff is not cleaned up and app will hang on re-launch

				return;
			}
		}

		if (g_engine.IsReady())
		{
			// Drawing is throttled to the screen update rate, so there
			// is no need to do timing here.
			g_engine.DrawFrame();
		}
	}
	LOGV("@@android_main, END...");
}
