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
};
struct ParameterWatcher 
{
    ParameterWatcher (tp::Parameters& parameters)
      : a (parameters.terrainModA), 
        b (parameters.terrainModB),
        c (parameters.terrainModC), 
        d (parameters.terrainModD), 
        index (parameters.currentTerrain)
    {}
    UBO getUBO() { return { (juce::roundToInt (index.getValue() * 4.0f)), 
                             a.getValue(), b.getValue(), c.getValue(), d.getValue()};}

private:
    struct WatchedParameter : private juce::AudioProcessorParameter::Listener
    {
        WatchedParameter (juce::AudioProcessorParameter* p)
          :  parameter (p)
        {
            parameter->addListener (this);
        }
        ~WatchedParameter() override { parameter->removeListener (this); }
        float getValue() { return value.load(); }
    private:
        juce::AudioProcessorParameter* parameter;
        std::atomic<float> value;
        void parameterValueChanged (int parameterIndex, float newValue) override
        {
            juce::ignoreUnused (parameterIndex);
            value.store (newValue);
        }
        virtual void parameterGestureChanged (int pi, bool gis) override { juce::ignoreUnused (pi, gis); }
    };
    WatchedParameter a, b, c, d, index;
};
class Visualizer : public juce::Component, 
                   private juce::OpenGLRenderer, 
                   private juce::Timer
{
public:
    Visualizer (tp::WaveTerrainSynthesizer& wts, tp::Parameters parameters)
      : camera (mutex), 
        parameterWatcher (parameters), 
        waveTerrainSynthesizer (wts)
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
        glContext.attachTo (*this);
    
        startTimerHz (60);
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
    }
private:
    juce::OpenGLContext glContext;
    juce::CriticalSection mutex;
    juce::Rectangle<int> bounds;
    Camera camera;
    std::unique_ptr<Terrain> terrain;
    ParameterWatcher parameterWatcher;
    std::unique_ptr<Trajectories> trajectories;
    tp::WaveTerrainSynthesizer& waveTerrainSynthesizer;

    void timerCallback() override 
    {
        glContext.triggerRepaint();
    }
    void newOpenGLContextCreated() override 
    {
        std::cout << juce::gl::glGetString (juce::gl::GL_VERSION) << std::endl;
        terrain = std::make_unique<Terrain> (glContext);
        trajectories = std::make_unique<Trajectories> (glContext, waveTerrainSynthesizer);
    }
    void renderOpenGL() override 
    {
        const juce::ScopedLock lock (mutex);
        juce::OpenGLHelpers::clear(juce::Colours::black);
        juce::gl::glClear (juce::gl::GL_COLOR_BUFFER_BIT | juce::gl::GL_DEPTH_BUFFER_BIT);
        auto desktopScale = static_cast<float>(glContext.getRenderingScale());
        juce::gl::glDepthFunc (juce::gl::GL_LESS);
        juce::gl::glEnable (juce::gl::GL_MULTISAMPLE);
        
        juce::gl::glViewport (0, 0, 
                              juce::roundToInt(desktopScale * static_cast<float>(bounds.getWidth())), 
                              juce::roundToInt(desktopScale * static_cast<float>(bounds.getHeight())));    
        auto ubo = parameterWatcher.getUBO();
        terrain->render(camera, ubo.index, ubo.a, ubo.b, ubo.c, ubo.d);
        trajectories->render (camera);
    }
    void openGLContextClosing() override 
    {
        terrain.reset();
        trajectories.reset();
    }
};