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
/**
 * @mainpage
 *
 * OVRWindow is an API that works in tandem with the Oculus SDK and the OpenGL
 * functionality provided by the Qt library to add support for the stereo rendering
 * model used by the Oculus Rift to Qt applications.
 */
#ifndef OVRWINDOW_H
#define OVRWINDOW_H
#define GL_GLEXT_PROTOTYPES

#include <OVR_CAPI.h>
#include <QWindow>
#include <QOpenGLFunctions>
#include <QMatrix4x4>


union ovrGLConfig;
union ovrGLTexture_s;
typedef ovrGLTexture_s ovrGLTexture;

class OVRWindow : public QWindow, protected QOpenGLFunctions {
Q_OBJECT
public:
    /**
     * TODO Explain me better.
     *
     * An enumeration of features that can be toggled. Be careful when disabling certain
     * features as it may induce simulator sickness.
     *
     * - LowPersistence eliminates motion blur and judder. Note that disabling this feature
     *   may induce simulator sickness.
     * - LatencyTesting ???
     * - DynamicPrediction ???
     *
     * - OrientationTracking tracks head orientation.
     * - YawCorrection ???
     * - PositionalTracking tracks the head's position.
     *
     * - ChromaticAberrationCorrection corrects chromatic aberration, a distortion caused by the
     *   Rift's lenses.
     * - Timewarp reduces motion-to-photon latency. Note that disabling this feature may induce
     *   simulator sickness.
     * - Vignette ???
     */
    enum class Feature : unsigned int {
        LowPersistence = ovrHmdCap_LowPersistence,
        LatencyTesting = ovrHmdCap_LatencyTest,
        DynamicPrediction = ovrHmdCap_DynamicPrediction,

        OrientationTracking = ovrSensorCap_Orientation,
        YawCorrection = ovrSensorCap_YawCorrection,
        PositionalTracking = ovrSensorCap_Position,

        ChromaticAberrationCorrection = ovrDistortionCap_Chromatic,
        Timewarp = ovrDistortionCap_TimeWarp,
        Vignette = ovrDistortionCap_Vignette,
    };
    /**
     * TODO Explain me.
     */
    enum class Vision {
        Monocular,
        Binocular
    };
    /**
     * TODO Explain me.
     */
    enum class LOD {
        Lowest,
        Low,
        Medium,
        High,
        Highest
    };
    /**
     * @struct FrameRenderContext
     * @brief A frame's render context.
     *
     * TODO Explain me.
     */
    struct FrameRenderContext {
        QMatrix4x4 view;
        struct {
            QMatrix4x4 perspective;
            QMatrix4x4 ortho;
        } projection;
    };
    /**
     * @brief Instantiate an OVRWindow object that is attached to an Oculus Rift device.
     *
     * The instantiated object is attached to a device with the specified index, and has
     * has a set of enabled features. If no hardware device is detected, a debug device
     * that emulates some of the DK1's features is used.
     *
     * @param index a positive integer used to access an Oculus Rift device.
     * @param features a set of device features to enable.
     */
    OVRWindow(const unsigned int& index, const std::initializer_list<OVRWindow::Feature>& features);
    /**
     * @brief Instantiate an OVRWindow object that is attached to an Oculus Rift device.
     *
     * The instantiated object is attached to a device with the index '0', and has all device
     * features enabled. If no hardware device is detected, a debug device that emulates some
     * of the DK1's features is used.
     */
    OVRWindow();
    /**
     * @brief The destructor.
     */
    virtual ~OVRWindow();
    /**
     * @brief Returns @c true if the OVRWindow has a valid OpenGL context, @c false otherwise.
     */
    bool hasValidGL() const;
    /**
     * @brief Return the OVRWindow's OpenGL context.
     */
    QOpenGLContext& getGL();
    /**
     * @brief Return the Oculus Rift's information.
     */
    const ovrHmdDesc& getDeviceInfo() const;
    /**
     * @brief Return a set of all enabled features.
     */
    const QSet<OVRWindow::Feature>& getEnabledFeatures() const;
    /**
     * Enable or disable a feature.
     * @param feature the feature to enable or disable.
     * @param enable true to enable the feature, false to disable it.
     */
    void enableFeature(const OVRWindow::Feature& feature, const bool enable = true);
    /**
     * Enable or disable a set of features.
     * @param features the features to enable or disable.
     * @param enable true to enable the features, false to disable them.
     */
    void enableFeatures(const std::initializer_list<OVRWindow::Feature>& features, const bool enable = true);
    /**
     * Returns true if the specified feature is enabled, false otherwise.
     * @param feature the feature to query.
     */
    bool isFeatureEnabled(const OVRWindow::Feature& feature) const;
    /**
     * Returns true if the specified feature is supported by the device, false otherwise.
     * @param feature the feature to query.
     */
    bool isFeatureSupported(const OVRWindow::Feature& feature) const;
    /**
     * Return the current vision mode.
     */
    const OVRWindow::Vision& getVision() const;
    /**
     * Set the vision.
     * @param vision the vision to set.
     */
    void setVision(const OVRWindow::Vision& vision);
    /**
     * Return the current level of detail.
     */
    const OVRWindow::LOD& getLOD() const;
    /**
     * Set the current level of detail (LOD). The LOD determines which features
     * are enabled or disabled with the goal of reducing frame render time,
     * thereby increasing performance. Note that unless dynamic LOD is disabled,
     * the LOD set by this member function will change.
     * @param lod the level of detail to set.
     */
    void setLOD(const OVRWindow::LOD& lod);
    /**
     * Enable or disable dynamic level of detail (LOD). Dynamic LOD adjusts the level
     * of detail to make sure the frame rate either matches, or is better than the
     * device's refresh rate.
     * @param enable true to enable dynamic LOD, false to disable.
     */
    void enableDynamicLOD(const bool enable = true);
    /**
     * Return the current interpupillary distance (IPD) in millimeters.
     */
    float getIPD() const;
    /**
     * Set the interpupillary distance (IPD) in millimeters.
     * @param ipd the distance to set.
     */
    void setIPD(const float& ipd);
    /**
     * TODO Explain me.
     */
    void forceZeroIPD(const bool& force);
    /**
     * TODO Explain me.
     */
    const float& getPixelDensity() const;
    /**
     * TODO Explain me.
     */
    void setPixelDensity(const float& density);
    /**
     * Return the viewing frustum's near clipping plane distance.
     */
    const float& getNearClippingDistance() const;
    /**
     * Set the viewing frustum's near clipping plane distance.
     * @param near the near clipping plane's distance.
     */
    void setNearClippingDistance(const float& near);
    /**
     * Return the viewing frustum's far clipping plane distance.
     */
    const float& getFarClippingDistance() const;
    /**
     * Set the viewing frustum's far clipping plane distance.
     * @param far the far clipping plane's distance.
     */
    void setFarClippingDistance(const float& far);
    /**
     * Returns true if multisampling is enabled, false otherwise.
     */
    bool isMultisamplingEnabled() const;
    /**
     * Enable or disable multisampling.
     * @param enable true to enable multisampling, false to disable.
     */
    void enableMultisampling(const bool enable = true);
protected:
    /**
     * @brief Initialize OpenGL.
     */
    virtual void initializeGL();
    /**
     * @brief This virtual function is called whenever a new frame needs to be rendered.
     */
    virtual void paintGL(const OVRWindow::FrameRenderContext& context, const float& dt);
    /**
     * @brief This virtual function is called whenever the window is resized.
     *
     * @param width the window's new width.
     * @param height the window's new height.
     */
    virtual void resizeGL(const unsigned int& width, const unsigned int& height);
    /**
     * Make the window's rendering context the current OpenGL context.
     */
    void makeCurrent();
    /**
     * Makes no GL context the current context. This may be useful in multi-threaded environments.
     */
    void doneCurrent();
    /**
     * @brief This virtual function is called whenever the level of detail (LOD) is changed.
     *
     * @param lod the new level of detail.
     */
    virtual void changeLOD(const OVRWindow::LOD& lod);
private:
    /**
     * Updates the window.
     */
    void updateGL();
    /**
     * Make a request to update the window by pushing an UpdateRequest
     * event on the event loop which will call updateGL later on.
     */
    void requestUpdateGL();
    /**
     * Configure the underlying OpenGL API for use with this interface.
     */
    void configureGL();
    /**
     * TODO Explain me.
     */
    void paintGL();
    /**
     * TODO Explain me.
     */
    ovrGLConfig& getOvrGlConfig() const;
    /**
     * TODO Explain me.
     */
    ovrGLTexture& getOvrGlTexture(const ovrEyeType& eye) const;
    /**
     * Update an outdated render target configuration.
     */
    void sanitizeRenderTargetConfiguration();
    /**
     * Update an outdated device configuration.
     */
    void sanitizeDeviceConfiguration();
    /**
     * Update an outdated rendering configuration.
     */
    void sanitizeRenderingConfiguration();
    /**
     * Adjust the LOD to make sure the frame rate matches the device's refresh rate.
     * @param dt the time since the last frame was rendered.
     */
    void adjustLOD(const float& dt, const unsigned int& tolerance);
    /**
     * @see QWindow::resizeEvent. This implementation of the resize event handler
     * is used to update OpenGL when the window is resized.
     */
    void resizeEvent(QResizeEvent* const) override final;
    /**
     * @see QWindow::exposeEvent. This implementation of the expose event handler
     * is used to initialize the OpenGL context when the window is first exposed.
     */
    void exposeEvent(QExposeEvent* const) override final;
    /**
     * @see QObject::event. This implementation of the generic event handler
     * is used to process update requests.
     */
    bool event(QEvent* const) override final;
    /**
     * Return the frame render context for a given eye.
     * @param eye the eye for which we wish to retrieve a frame render context.
     * @param pose the head pose.
     */
    const OVRWindow::FrameRenderContext& getFrameRenderContext(const ovrEyeType& eye, const ovrPosef& pose);
    /**
     * The device structure contains information about the device and its capabilities.
     */
    const ovrHmdDesc _device;
    /**
     * Sets of enabled features.
     */
    QSet<OVRWindow::Feature> _enabledFeatures;
    /**
     * This flag is set to true when dynamic LOD is enabled, false otherwise.
     */
    bool _enableDynamicLOD;
    /**
     * The OpenGL context.
     */
    QOpenGLContext _gl;
    /**
     * TODO Explain me.
     */
    bool _pendingUpdateRequest;
    /**
     * The render target which includes an FBO handle, texture handle and a resolution.
     */
    struct {
        GLuint fbo;
        GLuint pixel;
        GLuint depth;
        QSize resolution;
    } _renderTarget;
    /**
     * The field of view (FOV) for each eye.
     */
    ovrFovPort _FOV[ovrEye_Count];
    /**
     * The render information for each eye.
     */
    ovrEyeRenderDesc _renderInfo[ovrEye_Count];
    /**
     * The frame render context for each eye.
     */
    OVRWindow::FrameRenderContext _frameRenderContext[ovrEye_Count];
    /**
     * The viewing frustum's near clipping plane distance.
     */
    float _nearClippingPlaneDistance;
    /**
     * The viewing frustum's far clipping plane distance.
     */
    float _farClippingPlaneDistance;
    /**
     * TODO Explain me.
     */
    bool _forceZeroIPD;
    /**
     * TODO Explain me.
     */
    float _pixelDensity;
    /**
     * The vision mode.
     */
    OVRWindow::Vision _vision;
    /**
     * The interface's level of detail.
     */
    OVRWindow::LOD _LOD;
    /**
     * This set of variables keeps track of dirty configurations.
     */
    struct {
        bool renderTarget;
        bool rendering;
        struct { bool hmd, sensor; } device;
        bool projections[ovrEye_Count];
    } _dirty;
public slots:
    /**
     * @brief Toggle vision modes.
     */
    void toggleVision();
    /**
     * @brief Reduce the interface's level of detail.
     */
    void reduceLOD();
    /**
     * @brief Increase the interface's level of detail.
     */
    void increaseLOD();
    /**
     * @brief Toggle dynamic LOD.
     */
    void toggleDynamicLOD();
    /**
     * @brief Toggle multisampling.
     */
    void toggleMultisampling();
signals:
    /**
     * This signal is emitted when the interface has been correctly initialized and is ready for use.
     */
    void initialized();
    /**
     * This signal is emitted when the interface's level of detail (LOD) has been changed.
     * @param currentLOD the interface's current level of detail.
     */
    void LODChanged(const OVRWindow::LOD& currentLOD);
};

#endif // OVRWINDOW_H
