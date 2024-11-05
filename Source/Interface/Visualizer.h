#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>

#include "Renderer/Camera.h"
#include "Renderer/Terrain.h"
#include "Renderer/Trajectories.h"
#include "../Parameters.h"
#include "../DSP/WaveTerrainSynthesizer.h"
struct UBO
{
    int index;
    float a;
    float b;
    float c;
    float d;
    float saturation;
};
struct ParameterWatcher 
{
    ParameterWatcher (tp::Parameters& parameters)
      : a (parameters.terrainModA), 
        b (parameters.terrainModB),
        c (parameters.terrainModC), 
        d (parameters.terrainModD), 
        index (parameters.currentTerrain), 
        saturation (parameters.terrainSaturation)
    {}
    UBO getUBO() { return { juce::roundToInt ((index.getValue())), 
                            a.getValue(), b.getValue(), c.getValue(), d.getValue(),
                            saturation.getValue()};}

private:
    struct WatchedParameter : private juce::AudioProcessorParameter::Listener
    {
        WatchedParameter (juce::RangedAudioParameter* p)
          :  parameter (p)
        {
            parameter->addListener (this);
            value.store (parameter->convertFrom0to1 (parameter->getValue()));
        }
        ~WatchedParameter() override { parameter->removeListener (this); }
        float getValue() { return value.load(); }
    private:
        juce::RangedAudioParameter* parameter;
        std::atomic<float> value;
        void parameterValueChanged (int parameterIndex, float newValue) override
        {
            juce::ignoreUnused (parameterIndex);
            value.store (parameter->convertFrom0to1 (newValue));
        }
        virtual void parameterGestureChanged (int pi, bool gis) override { juce::ignoreUnused (pi, gis); }
    };
    WatchedParameter a, b, c, d, index, saturation;
};
class Visualizer : public juce::Component, 
                   private juce::OpenGLRenderer, 
                   private juce::Timer
{
public:
    Visualizer (tp::WaveTerrainSynthesizerStandard& wts, 
                tp::WaveTerrainSynthesizerMPE& wtsmpe, 
                tp::Parameters parameters)
      : camera (mutex), 
        parameterWatcher (parameters), 
        waveTerrainSynthesizerStandard (wts), 
        waveTerrainSynthesizerMPE (wtsmpe)
    {
#ifdef JUCE_MAC
        glContext.setOpenGLVersionRequired (juce::OpenGLContext::OpenGLVersion::openGL4_1);
#else
        glContext.setOpenGLVersionRequired (juce::OpenGLContext::OpenGLVersion::defaultGLVersion);
#endif
        glContext.setRenderer (this);

        juce::OpenGLPixelFormat pf;
        pf.multisamplingLevel = 4;
        glContext.setPixelFormat (pf);
        glContext.setMultisamplingEnabled (true);
        glContext.setComponentPaintingEnabled (false);
        glContext.setContinuousRepainting (true);

        glContext.attachTo (*this);
        // startTimerHz (60);
    }
    ~Visualizer() override 
    {
        glContext.detach();
    }
    void resized() override 
    {
        bounds = getLocalBounds();
        camera.setTargetBounds (bounds);
    }
    void mouseDown (const juce::MouseEvent& e) override 
    { 
        setMouseCursor (juce::MouseCursor::NoCursor);
        camera.mouseDown(e); 
    }
    void mouseDrag (const juce::MouseEvent& e) override { camera.mouseDrag(e); }
    void mouseWheelMove (const juce::MouseEvent& event,
                         const juce::MouseWheelDetails& wheel) override 
    { 
        juce::ignoreUnused (event);
        camera.mouseWheelMoved (wheel); 
    }
    void mouseUp (const juce::MouseEvent& e) override
    {
        juce::ignoreUnused (e);
        setMouseCursor (juce::MouseCursor::NormalCursor);
        juce::Point<int> b = this->getScreenPosition() + bounds.getCentre();
        juce::Desktop::setMousePosition (juce::Point<int>(b));
    }
private:
    juce::OpenGLContext glContext;
    juce::CriticalSection mutex;
    juce::Rectangle<int> bounds;
    Camera camera;
    std::unique_ptr<Terrain> terrain;
    ParameterWatcher parameterWatcher;
    std::unique_ptr<Trajectories> trajectories;
    tp::WaveTerrainSynthesizerStandard& waveTerrainSynthesizerStandard;
    tp::WaveTerrainSynthesizerMPE& waveTerrainSynthesizerMPE;

    void timerCallback() override 
    {
        glContext.triggerRepaint();
    }
    void newOpenGLContextCreated() override 
    {
        terrain = std::make_unique<Terrain> (glContext);
        trajectories = std::make_unique<Trajectories> (glContext, 
                                                       waveTerrainSynthesizerStandard,
                                                       waveTerrainSynthesizerMPE);
    }
    void renderOpenGL() override 
    {
        const juce::ScopedLock lock (mutex);
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        juce::OpenGLHelpers::clear(laf->getBackgroundDark());
        juce::gl::glClear (juce::gl::GL_COLOR_BUFFER_BIT | juce::gl::GL_DEPTH_BUFFER_BIT);
        auto desktopScale = static_cast<float>(glContext.getRenderingScale());
        juce::gl::glDepthFunc (juce::gl::GL_LESS);
        juce::gl::glEnable (juce::gl::GL_MULTISAMPLE);
        
        juce::gl::glViewport (0, 0, 
                              juce::roundToInt(desktopScale * static_cast<float>(bounds.getWidth())), 
                              juce::roundToInt(desktopScale * static_cast<float>(bounds.getHeight())));    
        auto ubo = parameterWatcher.getUBO();
        auto color = getLookAndFeel().findColour (juce::Slider::ColourIds::trackColourId);
        terrain->render(camera, color, ubo.index, ubo.a, ubo.b, ubo.c, ubo.d, ubo.saturation);
        color = getLookAndFeel().findColour (juce::Slider::ColourIds::thumbColourId);
        trajectories->render (camera, color);
    }
    void openGLContextClosing() override 
    {
        terrain.reset();
        trajectories.reset();
    }
};