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
// GLContext.cpp
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// includes
//--------------------------------------------------------------------------------
#include "GLContext.h"

#include <string.h>
#include <unistd.h>

#include "gl3stub.h"

namespace ndk_helper {

//--------------------------------------------------------------------------------
// eGLContext
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Ctor
//--------------------------------------------------------------------------------
GLContext::GLContext()
    : window_(nullptr),
      display_(EGL_NO_DISPLAY),
      surface_(EGL_NO_SURFACE),
      context_(EGL_NO_CONTEXT),
      screen_width_(0),
      screen_height_(0),
      gles_initialized_(false),
      egl_context_initialized_(false),
      es3_supported_(false) {}

void GLContext::InitGLES() {
  LOGV("@@GLContext::InitGLES, BEGIN...");
  if (gles_initialized_) return;
  //
  // Initialize OpenGL ES 3 if available
  //
  const char* versionStr = (const char*)glGetString(GL_VERSION);
  if (strstr(versionStr, "OpenGL ES 3.") && gl3stubInit()) {
    es3_supported_ = true;
    gl_version_ = 3.0f;
  } else {
    gl_version_ = 2.0f;
  }

  gles_initialized_ = true;
  LOGV("@@GLContext::InitGLES, END...");
}

//--------------------------------------------------------------------------------
// Dtor
//--------------------------------------------------------------------------------
GLContext::~GLContext() {
  LOGV("@@GLContext::~GLContext, BEGIN...");
  Terminate();
  LOGV("@@GLContext::~GLContext, END...");
}

bool GLContext::Init(ANativeWindow* window) {
  LOGV("@@GLContext::Init, BEGIN...");
  if (egl_context_initialized_)
  {
    LOGV("@@GLContext::Init, END...egl_context_initialized_");
    return true;
  }

  //
  // Initialize EGL
  //
  window_ = window;
  InitEGLSurface();
  InitEGLContext();
  InitGLES();

  egl_context_initialized_ = true;

  LOGV("@@GLContext::Init, END...");
  return true;
}

bool GLContext::InitEGLSurface() {
  LOGV("@@GLContext::InitEGLSurface, BEGIN...");
  display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglInitialize(display_, 0, 0);

  /*
   * Here specify the attributes of the desired configuration.
   * Below, we select an EGLConfig with at least 8 bits per color
   * component compatible with on-screen windows
   */
  const EGLint attribs[] = {EGL_RENDERABLE_TYPE,
                            EGL_OPENGL_ES2_BIT,  // Request opengl ES2.0
                            EGL_SURFACE_TYPE,
                            EGL_WINDOW_BIT,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_RED_SIZE,
                            8,
                            EGL_DEPTH_SIZE,
                            24,
                            EGL_NONE};
  color_size_ = 8;
  depth_size_ = 24;

  EGLint num_configs;
  eglChooseConfig(display_, attribs, &config_, 1, &num_configs);

  if (!num_configs) {
    // Fall back to 16bit depth buffer
    const EGLint attribs[] = {EGL_RENDERABLE_TYPE,
                              EGL_OPENGL_ES2_BIT,  // Request opengl ES2.0
                              EGL_SURFACE_TYPE,
                              EGL_WINDOW_BIT,
                              EGL_BLUE_SIZE,
                              8,
                              EGL_GREEN_SIZE,
                              8,
                              EGL_RED_SIZE,
                              8,
                              EGL_DEPTH_SIZE,
                              16,
                              EGL_NONE};
    eglChooseConfig(display_, attribs, &config_, 1, &num_configs);
    depth_size_ = 16;
  }

  if (!num_configs) {
    LOGW("Unable to retrieve EGL config");
    return false;
  }

  surface_ = eglCreateWindowSurface(display_, config_, window_, NULL);
  eglQuerySurface(display_, surface_, EGL_WIDTH, &screen_width_);
  eglQuerySurface(display_, surface_, EGL_HEIGHT, &screen_height_);

  LOGV("@@GLContext::InitEGLSurface, END...");
  return true;
}

bool GLContext::InitEGLContext() {
  LOGV("@@GLContext::InitEGLContext, BEGIN...");
  const EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION,
                                    2,  // Request opengl ES2.0
                                    EGL_NONE};
  context_ = eglCreateContext(display_, config_, NULL, context_attribs);

  if (eglMakeCurrent(display_, surface_, surface_, context_) == EGL_FALSE) {
    LOGW("Unable to eglMakeCurrent");
    return false;
  }

  context_valid_ = true;
  LOGV("@@GLContext::InitEGLContext, END...");
  return true;
}

EGLint GLContext::Swap() {
  LOGV("@@GLContext::Swap, BEGIN...");
  EGLSurface surface_cur_ = eglGetCurrentSurface(EGL_DRAW);
  bool b = eglSwapBuffers(display_, surface_cur_);//bool b = eglSwapBuffers(display_, surface_);//FIXME!
  if (!b) {
    EGLint err = eglGetError();
    if (err == EGL_BAD_SURFACE) {
      // Recreate surface
      InitEGLSurface();
      LOGV("@@GLContext::Swap, END...Recreate");
      return EGL_SUCCESS;  // Still consider glContext is valid
    } else if (err == EGL_CONTEXT_LOST || err == EGL_BAD_CONTEXT) {
      // Context has been lost!!
      context_valid_ = false;
      Terminate();
      InitEGLContext();
    }
    LOGV("@@GLContext::Swap, END...err");
    return err;
  }
  LOGV("@@GLContext::Swap, END...");
  return EGL_SUCCESS;
}

void GLContext::Terminate() {
  LOGV("@@GLContext::Terminate, BEGIN...");
  if (display_ != EGL_NO_DISPLAY) {
    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (context_ != EGL_NO_CONTEXT) {
      eglDestroyContext(display_, context_);
    }

    if (surface_ != EGL_NO_SURFACE) {
      eglDestroySurface(display_, surface_);
    }
    eglTerminate(display_);
  }

  display_ = EGL_NO_DISPLAY;
  context_ = EGL_NO_CONTEXT;
  surface_ = EGL_NO_SURFACE;
  window_ = nullptr;
  context_valid_ = false;
  LOGV("@@GLContext::Terminate, END...");
}

EGLint GLContext::Resume(ANativeWindow* window) {
  LOGV("@@GLContext::Resume, BEGIN...");
  if (egl_context_initialized_ == false) {
    Init(window);
    LOGV("@@GLContext::Resume, END...if (egl_context_initialized_ == false)");
    return EGL_SUCCESS;
  }

  int32_t original_widhth = screen_width_;
  int32_t original_height = screen_height_;

  // Create surface
  window_ = window;
  surface_ = eglCreateWindowSurface(display_, config_, window_, NULL);
  eglQuerySurface(display_, surface_, EGL_WIDTH, &screen_width_);
  eglQuerySurface(display_, surface_, EGL_HEIGHT, &screen_height_);

  if (screen_width_ != original_widhth || screen_height_ != original_height) {
    // Screen resized
    LOGI("Screen resized");
  }

  if (eglMakeCurrent(display_, surface_, surface_, context_) == EGL_TRUE)
  {
    LOGV("@@GLContext::Resume, END...eglMakeCurrent");
    return EGL_SUCCESS;
  }

  EGLint err = eglGetError();
  LOGW("Unable to eglMakeCurrent %d", err);

  if (err == EGL_CONTEXT_LOST) {
    // Recreate context
    LOGI("Re-creating egl context");
    InitEGLContext();
  } else {
    // Recreate surface
    Terminate();
    InitEGLSurface();
    InitEGLContext();
  }

  LOGV("@@GLContext::Resume, END...");
  return err;
}

void GLContext::Suspend() {
  LOGV("@@GLContext::Suspend, BEGIN...");
  if (surface_ != EGL_NO_SURFACE) {
    eglDestroySurface(display_, surface_);
    surface_ = EGL_NO_SURFACE;
  }
  LOGV("@@GLContext::Suspend, END...");
}

bool GLContext::Invalidate() {
  LOGV("@@GLContext::Invalidate, BEGIN...");
  Terminate();

  egl_context_initialized_ = false;
  LOGV("@@GLContext::Invalidate, END...");
  return true;
}

bool GLContext::CheckExtension(const char* extension) {
  LOGV("@@GLContext::CheckExtension, BEGIN...");
  if (extension == NULL)
  {
    LOGV("@@GLContext::CheckExtension, END...if (extension == NULL)");
    return false;
  }

  std::string extensions = std::string((char*)glGetString(GL_EXTENSIONS));
  std::string str = std::string(extension);
  str.append(" ");

  size_t pos = 0;
  if (extensions.find(extension, pos) != std::string::npos) {
    LOGV("@@GLContext::CheckExtension, END...return true;");
    return true;
  }

  LOGV("@@GLContext::CheckExtension, END...");
  return false;
}

}  // namespace ndkHelper
