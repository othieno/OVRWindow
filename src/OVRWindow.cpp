/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Jeremy Othieno.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "OVRWindow.h"
#include <OVR.h>
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <QResizeEvent>
#include <QExposeEvent>
#include <QMap>
#include <cassert>
#include <atomic>
#if defined(Q_OS_LINUX)
#define OVR_OS_LINUX
#elif defined(Q_OS_MAC)
#define OVR_OS_MAC
#elif defined(Q_OS_WIN32)
#define OVR_OS_WIN32
#endif
#include <OVR_CAPI_GL.h>


// This is a required function definition for QSet<OVRWindow::Feature> to be used.
uint qHash(const OVRWindow::Feature& feature)
{
   // Make sure OVRWindow::Feature's underlying type is an unsigned integer.
   assert((std::is_same<uint, std::underlying_type<OVRWindow::Feature>::type>::value));
   return static_cast<uint>(feature);
}


OVRWindow::OVRWindow(const unsigned int& index, const std::initializer_list<OVRWindow::Feature>& features) :
QWindow(static_cast<QScreen*>(nullptr)),
_device(),
_enableDynamicLOD(false),
_pendingUpdateRequest(false),
_renderTarget({0, 0, 0, QSize(0, 0)}),
_nearClippingPlaneDistance(0.01f),
_farClippingPlaneDistance(10000.0f),
_forceZeroIPD(false),
_pixelDensity(1.0f),
_vision(OVRWindow::Vision::Binocular),
_LOD(OVRWindow::LOD::Highest),
_dirty({true, true, {true, true}, {true, true}})
{
   // Only one instance of this class can be created.
   static std::atomic<bool> OVRWINDOW_INSTANTIATED(false);
   assert(!OVRWINDOW_INSTANTIATED);
   OVRWINDOW_INSTANTIATED = true;

   // Make sure the windowing system has OpenGL support.
   setSurfaceType(QWindow::OpenGLSurface);
   assert(supportsOpenGL());

   // Initialize LibOVR and make sure the device index is valid.
   ovr_Initialize();
   assert(!index || index < static_cast<unsigned int>(ovrHmd_Detect()));

   // Initialize the HMD device. If no device is detected, create a debug device.
   auto hmd = ovrHmd_Create(index);
   if (!hmd)
      hmd = ovrHmd_CreateDebug(ovrHmd_DK1);
   ovrHmd_GetDesc(hmd, const_cast<ovrHmdDesc*>(&_device));

   // Initialize the FOV parameters.
   std::copy(std::begin(_device.DefaultEyeFov), std::end(_device.DefaultEyeFov), _FOV);

   // Enable features.
   enableFeatures(features);
}


OVRWindow::OVRWindow() :
OVRWindow(0,
{
   OVRWindow::Feature::LowPersistence,
   OVRWindow::Feature::LatencyTesting,
   OVRWindow::Feature::DynamicPrediction,
   OVRWindow::Feature::OrientationTracking,
   OVRWindow::Feature::YawCorrection,
   OVRWindow::Feature::PositionalTracking,
   OVRWindow::Feature::ChromaticAberrationCorrection,
   OVRWindow::Feature::Timewarp,
   OVRWindow::Feature::Vignette
})
{}


OVRWindow::~OVRWindow()
{
   if (_renderTarget.pixel != 0)
      glDeleteTextures(1, &_renderTarget.pixel);

   //FIXME Find out why these cause a segmentation fault in Qt5.
   //if (_renderTarget.depth != 0)
      //glDeleteRenderbuffers(1, &_renderTarget.depth);

//   if (_renderTarget.fbo != 0)
//      glDeleteFramebuffers(1, &_renderTarget.fbo);

   // Destroy the device and shutdown LibOVR.
   ovrHmd_Destroy(_device.Handle);
   ovr_Shutdown();
}


void
OVRWindow::initializeGL()
{}


void
OVRWindow::resizeGL(const unsigned int&, const unsigned int&)
{}


void
OVRWindow::paintGL(const OVRWindow::FrameRenderContext&, const float&)
{}


bool
OVRWindow::hasValidGL() const
{
   return _gl.isValid();
}


QOpenGLContext&
OVRWindow::getGL()
{
   return _gl;
}


void
OVRWindow::makeCurrent()
{
   const auto& result = _gl.makeCurrent(this);
   assert(result);
}


void
OVRWindow::doneCurrent()
{
   _gl.doneCurrent();
}


const ovrHmdDesc&
OVRWindow::getDeviceInfo() const
{
   return _device;
}


const QSet<OVRWindow::Feature>&
OVRWindow::getEnabledFeatures() const
{
   return _enabledFeatures;
}


void
OVRWindow::enableFeature(const OVRWindow::Feature& feature, const bool enable)
{
   // If the feature is already enabled and a request to enable it is made, then the
   // request is ignored. Likewise, if a feature is disabled and a request to disable
   // it is made, then the request is ignored. If a feature is enabled or disabled,
   // then the render configuration must be updated.
   if (enable != isFeatureEnabled(feature) && isFeatureSupported(feature))
   {
      if (enable)
         _enabledFeatures << feature;
      else
         _enabledFeatures.remove(feature);

      // Mark the configuration as dirty.
      switch (feature)
      {
         case OVRWindow::Feature::LowPersistence:
         case OVRWindow::Feature::LatencyTesting:
         case OVRWindow::Feature::DynamicPrediction:
            _dirty.device.hmd = true;
         break;
         case OVRWindow::Feature::OrientationTracking:
         case OVRWindow::Feature::YawCorrection:
         case OVRWindow::Feature::PositionalTracking:
            _dirty.device.sensor = true;
         break;
         case OVRWindow::Feature::ChromaticAberrationCorrection:
         case OVRWindow::Feature::Timewarp:
         case OVRWindow::Feature::Vignette:
            _dirty.rendering = true;
         break;
         default:
            // Make sure all features are accounted for (incase OVRWindow::Feature gets modified in the future).
            assert(false);
         break;
      }
   }
}


void
OVRWindow::enableFeatures(const std::initializer_list<OVRWindow::Feature>& features, const bool enable)
{
   for (const auto& feature : features)
      enableFeature(feature, enable);
}


bool
OVRWindow::isFeatureEnabled(const OVRWindow::Feature& feature) const
{
   return _enabledFeatures.contains(feature);
}


bool
OVRWindow::isFeatureSupported(const OVRWindow::Feature& feature) const
{
   const auto& mask = static_cast<std::underlying_type<OVRWindow::Feature>::type>(feature);
   switch (feature)
   {
      // These features are not supported on all revisions of the Rift HMD.
      case OVRWindow::Feature::PositionalTracking:
      case OVRWindow::Feature::YawCorrection:
         return (mask & _device.SensorCaps) == _device.SensorCaps;
      default:
         return true;
   }
}


const OVRWindow::Vision&
OVRWindow::getVision() const
{
   return _vision;
}


void
OVRWindow::setVision(const OVRWindow::Vision& vision)
{
   if (_vision != vision)
   {
      _vision = vision;
      _dirty.rendering = true;
   }
}


void
OVRWindow::toggleVision()
{
   setVision(_vision != OVRWindow::Vision::Binocular ? OVRWindow::Vision::Binocular : OVRWindow::Vision::Monocular);
}


const OVRWindow::LOD&
OVRWindow::getLOD() const
{
   return _LOD;
}


void
OVRWindow::setLOD(const OVRWindow::LOD& lod)
{
   if (_LOD != lod)
   {
      _LOD = lod;
      changeLOD(_LOD);
      emit LODChanged(_LOD);
   }
}


void
OVRWindow::changeLOD(const OVRWindow::LOD& lod)
{
   // Reset enabled features.
   _enabledFeatures.clear();

   QSet<OVRWindow::Feature> features;
   switch (lod)
   {
      case OVRWindow::LOD::Highest:
         setPixelDensity(1.5f);
      break;
      case OVRWindow::LOD::High:
         setPixelDensity(1.0f);
      break;
      case OVRWindow::LOD::Medium:
         setPixelDensity(1.0f);
      break;
      case OVRWindow::LOD::Low:
         setPixelDensity(0.5f);
         enableMultisampling(false);
      break;
      case OVRWindow::LOD::Lowest:
         setPixelDensity(0.25f);
         enableMultisampling(false);
      break;
      default:
         assert(false);
      break;
   }
   // Enable the new feature set.
   for (const auto& feature : features)
      enableFeature(feature, true);
}


void
OVRWindow::enableDynamicLOD(const bool enable)
{
   _enableDynamicLOD = enable;
}


void
OVRWindow::reduceLOD()
{
   if (_LOD != OVRWindow::LOD::Lowest)
      setLOD(static_cast<OVRWindow::LOD>(static_cast<std::underlying_type<OVRWindow::LOD>::type>(_LOD) - 1));
}


void
OVRWindow::increaseLOD()
{
   if (_LOD != OVRWindow::LOD::Highest)
      setLOD(static_cast<OVRWindow::LOD>(static_cast<std::underlying_type<OVRWindow::LOD>::type>(_LOD) + 1));
}


void
OVRWindow::toggleDynamicLOD()
{
   _enableDynamicLOD = !_enableDynamicLOD;
}


float
OVRWindow::getIPD() const
{
   return ovrHmd_GetFloat(_device.Handle, OVR_KEY_IPD, OVR_DEFAULT_IPD);
}


void
OVRWindow::setIPD(const float& ipd)
{
   // If the IPD is changed, then the render configuration needs to be updated.
   _dirty.rendering = ovrHmd_SetFloat(_device.Handle, OVR_KEY_IPD, ipd);
}


void
OVRWindow::forceZeroIPD(const bool& force)
{
   // If the IPD is changed, then the render configuration needs to be updated.
   if (_forceZeroIPD != force)
   {
      _forceZeroIPD = force;
      _dirty.rendering = true;
   }
}


const float&
OVRWindow::getPixelDensity() const
{
   return _pixelDensity;
}


void
OVRWindow::setPixelDensity(const float& density)
{
   // When the pixel density is changed, the render target needs to be resized.
   if (_pixelDensity != density)
   {
      _pixelDensity = density <= 0.0f ? 0.5f : density;
      _dirty.renderTarget = true;
   }
}


const float&
OVRWindow::getNearClippingDistance() const
{
   return _nearClippingPlaneDistance;
}


void
OVRWindow::setNearClippingDistance(const float& near)
{
   if (_nearClippingPlaneDistance != near)
   {
      _nearClippingPlaneDistance = near;
      for (auto& dirty : _dirty.projections)
         dirty = true;
   }
}


const float&
OVRWindow::getFarClippingDistance() const
{
   return _farClippingPlaneDistance;
}


void
OVRWindow::setFarClippingDistance(const float& far)
{
   if (_farClippingPlaneDistance != far)
   {
      _farClippingPlaneDistance = far;
      for (auto& dirty : _dirty.projections)
         dirty = true;
   }
}


bool
OVRWindow::isMultisamplingEnabled() const
{
   return getOvrGlConfig().OGL.Header.Multisample == 1;
}


void
OVRWindow::enableMultisampling(const bool enable)
{
   auto& multisample = getOvrGlConfig().OGL.Header.Multisample;
   if (static_cast<bool>(multisample) != enable)
   {
      multisample = enable ? 1 : 0;
      _dirty.rendering = true;
   }
}


void
OVRWindow::toggleMultisampling()
{
   auto& multisample = getOvrGlConfig().OGL.Header.Multisample;
   multisample = !multisample;
   _dirty.rendering = true;
}


void
OVRWindow::updateGL()
{
   if (isExposed() && hasValidGL())
   {
      makeCurrent();
      paintGL();
      doneCurrent();
      requestUpdateGL();
   }
}


void
OVRWindow::requestUpdateGL()
{
   if (!_pendingUpdateRequest)
   {
      _pendingUpdateRequest = true;
      QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
   }
}


void
OVRWindow::configureGL()
{
   auto& OGL = getOvrGlConfig().OGL;

   OGL.Header.API = ovrRenderAPI_OpenGL;
   OGL.Header.RTSize.w = _device.Resolution.w;
   OGL.Header.RTSize.h = _device.Resolution.h;
   OGL.Header.Multisample = 0;
#if defined(Q_OS_LINUX)
   auto* const display =
   QGuiApplication::platformNativeInterface()->nativeResourceForWindow(QByteArray("display"), this);
   assert(display != nullptr);

   OGL.Disp = static_cast<::Display*>(display);
   OGL.Win = static_cast<::Window>(winId());
#elif defined(Q_OS_WIN32)
   __OVR_GL_CONFIG.Window = static_cast<::HWND>(winId());
#endif
   _dirty.rendering = true;
}


void
OVRWindow::paintGL()
{
   // Update all configurations before drawing the frame.
   sanitizeRenderTargetConfiguration();
   sanitizeDeviceConfiguration();
   sanitizeRenderingConfiguration();

   const auto& hmd = _device.Handle;
   const auto& frameTiming = ovrHmd_BeginFrame(hmd, 0);
   const auto& dt = frameTiming.DeltaSeconds;

   adjustLOD(dt, 5);

   glBindFramebuffer(GL_FRAMEBUFFER, _renderTarget.fbo);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   for (const auto& eye : _device.EyeRenderOrder)
   {
            auto& texConfig = getOvrGlTexture(eye);
      const auto& viewport = texConfig.OGL.Header.RenderViewport;
      const auto& pose = ovrHmd_BeginEyeRender(hmd, eye);
      const auto& frameRenderContext = getFrameRenderContext(eye, pose);

      glViewport(viewport.Pos.x, viewport.Pos.y, viewport.Size.w, viewport.Size.h);
      paintGL(frameRenderContext, dt);
      ovrHmd_EndEyeRender(hmd, eye, pose, &texConfig.Texture);
   }
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   ovrHmd_EndFrame(hmd);

   // TODO Remove this block when ovrHmd_EndFrame cleans up after itself correctly.
   {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glUseProgram(0);
   }
}


ovrGLConfig&
OVRWindow::getOvrGlConfig() const
{
   static ovrGLConfig INSTANCE;
   return INSTANCE;
}


ovrGLTexture&
OVRWindow::getOvrGlTexture(const ovrEyeType& eye) const
{
   static std::array<ovrGLTexture, ovrEye_Count> INSTANCES;
   return INSTANCES[eye];
}


void
OVRWindow::sanitizeRenderTargetConfiguration()
{
   // Reconfigure the render target.
   if (_dirty.renderTarget)
   {
      bool isInitialized = _renderTarget.fbo;
      if (!isInitialized)
      {
         // Initialize the frame buffer object and its textures.
         assert(hasValidGL());
         glGenFramebuffers(1, &_renderTarget.fbo);
         assert(_renderTarget.fbo != 0);

         glGenTextures(1, &_renderTarget.pixel);
         assert(_renderTarget.pixel != 0);

         glGenRenderbuffers(1, &_renderTarget.depth);
         assert(_renderTarget.depth != 0);

         // Setup the platform-independent configuration to use the OpenGL API
         // during the SDK distortion correction phase.
         for (const auto& eye : _device.EyeRenderOrder)
            getOvrGlTexture(eye).OGL.Header.API = ovrRenderAPI_OpenGL;
      }
      glBindFramebuffer(GL_FRAMEBUFFER, _renderTarget.fbo);

      const auto& hmd = _device.Handle;
      const auto& sizeL = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left,  _FOV[ovrEye_Left],  _pixelDensity);
      const auto& sizeR = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, _FOV[ovrEye_Right], _pixelDensity);
      const auto& newSize = QSize(sizeL.w + sizeR.w, std::max(sizeL.h, sizeR.h));
      if (_renderTarget.resolution != newSize)
      {
         _renderTarget.resolution = newSize;
         const auto& w = newSize.width();
         const auto& h = newSize.height();

         // Bind and resize the buffers.
         glBindTexture(GL_TEXTURE_2D, _renderTarget.pixel);
         glBindRenderbuffer(GL_RENDERBUFFER, _renderTarget.depth);
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
         glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);

         // If the framebuffer object was just initialized, configure each buffer appropriately.
         if (!isInitialized)
         {
            // Configure the pixel buffer.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _renderTarget.pixel, 0);
            const GLenum& buffers = GL_COLOR_ATTACHMENT0;
            glDrawBuffers(1, &buffers);

            // Configure the depth buffer.
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _renderTarget.depth);

            // Make sure the framebuffer object is valid.
            assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
         }
         glBindTexture(GL_TEXTURE_2D, 0);
         glBindRenderbuffer(GL_RENDERBUFFER, 0);

         // Configure SDK distortion correction parameters.
         for (unsigned int i = 0; i < ovrEye_Count; ++i)
         {
            auto& ogl = getOvrGlTexture(static_cast<ovrEyeType>(i)).OGL;
            auto& header = ogl.Header;

            ogl.TexId = _renderTarget.pixel;
            header.TextureSize.w = w;
            header.TextureSize.h = h;
            header.RenderViewport.Pos.x = i * ((w + 1) * 0.5);
            header.RenderViewport.Pos.y = 0;
            header.RenderViewport.Size.w = w * 0.5f;
            header.RenderViewport.Size.h = h;
         }
         // Mark the rendering configuration as dirty since the render target has been resized.
         _dirty.rendering = true;
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      // Mark the render target as sanitized.
      _dirty.renderTarget = false;
   }
}


void
OVRWindow::sanitizeDeviceConfiguration()
{
   static const auto& getHmdCaps = [this]()
   {
      const auto& features =
      {
         OVRWindow::Feature::LowPersistence,
         OVRWindow::Feature::LatencyTesting,
         OVRWindow::Feature::DynamicPrediction
      };
      unsigned int caps = 0;
      for (const auto& feature : features)
      {
         if (isFeatureEnabled(feature))
            caps |= static_cast<std::underlying_type<OVRWindow::Feature>::type>(feature);
      }
      return caps;
   };
   static const auto& getSensorCaps = [this]()
   {
      const auto& features =
      {
         OVRWindow::Feature::LowPersistence,
         OVRWindow::Feature::LatencyTesting,
         OVRWindow::Feature::DynamicPrediction
      };
      unsigned int caps = 0;
      for (const auto& feature : features)
      {
         if (isFeatureEnabled(feature))
            caps |= static_cast<std::underlying_type<OVRWindow::Feature>::type>(feature);
      }
      return caps;
   };
   const auto& hmd = _device.Handle;
   if (_dirty.device.hmd)
   {
      ovrHmd_SetEnabledCaps(hmd, getHmdCaps());

      // Mark the HMD's configuration as sanitized. Note that a change to some of the
      // HMD's capabilities modifies the rendering configuration.
      _dirty.device.hmd = false;
      _dirty.rendering = true;
   }
   if (_dirty.device.sensor)
   {
      // If no sensor capability is activated, stop the sensor.
      const auto& sensorCaps = getSensorCaps();
      if (sensorCaps) {
         const auto result = ovrHmd_StartSensor(hmd, _device.SensorCaps, sensorCaps);
         assert(result);
      }
      else
         ovrHmd_StopSensor(hmd);

      // Mark the sensor's configuration as sanitized.
      _dirty.device.sensor = false;
   }
}


void
OVRWindow::sanitizeRenderingConfiguration()
{
   static const auto& getDistortionCaps = [this]()
   {
      const auto& features =
      {
         OVRWindow::Feature::ChromaticAberrationCorrection,
         OVRWindow::Feature::Timewarp,
         OVRWindow::Feature::Vignette
      };
      unsigned int caps = 0;
      for (const auto& feature : features)
      {
         if (isFeatureEnabled(feature))
            caps |= static_cast<std::underlying_type<OVRWindow::Feature>::type>(feature);
      }
      return caps;
   };
   const auto& hmd = _device.Handle;
   if (_dirty.rendering)
   {
      const auto result = ovrHmd_ConfigureRendering(hmd, &getOvrGlConfig().Config, getDistortionCaps(), _FOV, _renderInfo);
      assert(result);
      if (_forceZeroIPD)
      {
         for (auto& info : _renderInfo)
            info.ViewAdjust = OVR::Vector3f(0);
      }
      // Mark the rendering configuration as sanitized.
      _dirty.rendering = false;
   }
}


void
OVRWindow::adjustLOD(const float& dt, const unsigned int& tolerance)
{
   static const QMap<ovrHmdType, unsigned int> REFRESH_RATES =
   {
      {ovrHmd_DK1, 60},
      {ovrHmd_DK2, 75}
   };
   static const unsigned int TARGET_FPS = REFRESH_RATES[REFRESH_RATES.contains(_device.Type) ? _device.Type : ovrHmd_DK1];
   static const unsigned int MAX_FPS_SAMPLES = 32;
   static unsigned int NSAMPLES = 0;
   static float ACCUMULATOR = 0;
   static float FPS = 0;

   ACCUMULATOR += 1.0f / dt;
   ++NSAMPLES;

   // Calculate average FPS.
   if (NSAMPLES == MAX_FPS_SAMPLES)
   {
      FPS = ACCUMULATOR / static_cast<float>(NSAMPLES);
      if (_enableDynamicLOD && _LOD != OVRWindow::LOD::Lowest)
      {
         const int dFPS = TARGET_FPS - static_cast<unsigned int>(std::round(FPS));
         if (dFPS > static_cast<int>(tolerance))
            reduceLOD();
      }
      NSAMPLES = 0;
      ACCUMULATOR = 0;
   }
}


void
OVRWindow::resizeEvent(QResizeEvent* const e)
{
   if (_gl.isValid())
   {
      makeCurrent();
      const auto& newSize = e->size();
      resizeGL(newSize.width(), newSize.height());
      doneCurrent();
   }
   QWindow::resizeEvent(e);
}


void
OVRWindow::exposeEvent(QExposeEvent* const e)
{
   // When the window is exposed the first time, it needs to be initialized.
   static bool isInitialized = false;
   if (!isInitialized && isExposed())
   {
      _gl.setFormat(requestedFormat());
      auto result = _gl.create();
      assert(result);
      makeCurrent();
      assert(hasValidGL());
      initializeOpenGLFunctions();
      initializeGL();
      configureGL();
      doneCurrent();
      requestUpdateGL();
      isInitialized = true;
      emit initialized();
   }
   QWindow::exposeEvent(e);
}


bool
OVRWindow::event(QEvent* const e)
{
   if (e->type() == QEvent::UpdateRequest)
   {
      _pendingUpdateRequest = false;
      updateGL();
      return true;
   }
   return QWindow::event(e);
}


const OVRWindow::FrameRenderContext&
OVRWindow::getFrameRenderContext(const ovrEyeType& eye, const ovrPosef& pose)
{
         auto& frameRenderContext = _frameRenderContext[eye];
   const auto& renderInfo = _renderInfo[eye];
   const auto& viewAdjust = renderInfo.ViewAdjust;

   // Calculate the view matrix.
   const auto& viewMatrix = (
               OVR::Matrix4f::Translation(viewAdjust) *
               OVR::Matrix4f(OVR::Quatf(pose.Orientation).Inverted())
               );
   const auto* V = &viewMatrix.M[0][0];
   for (unsigned int i = 0; i < 4; ++i)
   {
      auto& view = frameRenderContext.view;
      view(i, 0) = *V++;
      view(i, 1) = *V++;
      view(i, 2) = *V++;
      view(i, 3) = *V++;
   }

   // Calculate the projection matrices, if need be.
   auto& dirtyProjection = _dirty.projections[eye];
   if (dirtyProjection)
   {
      // Calculate the perspective projection.
      const auto& znear = _nearClippingPlaneDistance;
      const auto& zfar = _farClippingPlaneDistance;
      const auto& perspective = ovrMatrix4f_Projection(renderInfo.Fov, znear, zfar, true);

      // Calculate the orthogonal (orthographic) projection.
      const auto& distance = 0.8f; // 2D is 0.8 meters from the camera.
      const auto& scale = OVR::Vector2f(1.0f) / OVR::Vector2f(renderInfo.PixelsPerTanAngleAtCenter);
      const auto& ortho = ovrMatrix4f_OrthoSubProjection(perspective, scale, distance, viewAdjust.x);

      // Convert OVR::Matrix4f to QMatrix4x4.
      const auto* P = &perspective.M[0][0];
      const auto* O = &ortho.M[0][0];
      for (unsigned int i = 0; i < 4; ++i)
      {
         auto& perspective = frameRenderContext.projection.perspective;
         perspective(i, 0) = *P++;
         perspective(i, 1) = *P++;
         perspective(i, 2) = *P++;
         perspective(i, 3) = *P++;

         auto& ortho = frameRenderContext.projection.ortho;
         ortho(i, 0) = *O++;
         ortho(i, 1) = *O++;
         ortho(i, 2) = *O++;
         ortho(i, 3) = *O++;
      }
      dirtyProjection = false;
   }
   return frameRenderContext;
}
