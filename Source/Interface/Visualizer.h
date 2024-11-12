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
    UBO getUBO() const { return { juce::roundToInt ((index.getValue())), 
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
        float getValue() const { return value.load(); }
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
struct MPEWatcher : private juce::ValueTree::Listener
{
    MPEWatcher (juce::ValueTree mpeVoices, 
                juce::AudioProcessorValueTreeState& apvts)
      : voicesState (mpeVoices), 
        valueTreeState (apvts)
    {
        checkIfControlled();
        valueTreeState.state.addListener (this);
    }
    ~MPEWatcher() { valueTreeState.state.removeListener (this); }

    bool aControlled() const { return channelsData[0].isControlled; }
    bool bControlled() const { return channelsData[1].isControlled; }
    bool cControlled() const { return channelsData[2].isControlled; }
    bool dControlled() const { return channelsData[3].isControlled; }
    bool saturationControlled() const { return channelsData[4].isControlled; }
    juce::Array<float> getArrayA() const
    {
        juce::Array<float> a;
        int index = 0;
        for (int i = 0; i < voicesState.getNumChildren(); i++)
        {
            float x = 0;
            if (channelsData[index].voiceChannel == id::PRESSURE) x = voicesState.getChild (i).getProperty (id::voicePressure);
            if (channelsData[index].voiceChannel == id::TIMBRE) x = voicesState.getChild (i).getProperty (id::voiceTimbre);
            a.add (curveValue (x,
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::curve),
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::handleOne),
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::handleTwo)));
        }
        return a;
    }
    juce::Array<float> getArrayB() const
    {
        juce::Array<float> a;
        int index = 1;
        for (int i = 0; i < voicesState.getNumChildren(); i++)
        {
            float x = 0;
            if (channelsData[index].voiceChannel == id::PRESSURE) x = voicesState.getChild (i).getProperty (id::voicePressure);
            if (channelsData[index].voiceChannel == id::TIMBRE) x = voicesState.getChild (i).getProperty (id::voiceTimbre);
            a.add (curveValue (x,
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::curve),
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::handleOne),
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::handleTwo)));
        }
        return a;
    }
    juce::Array<float> getArrayC() const
    {
        juce::Array<float> a;
        int index = 2;
        for (int i = 0; i < voicesState.getNumChildren(); i++)
        {
            float x = 0;
            if (channelsData[index].voiceChannel == id::PRESSURE) x = voicesState.getChild (i).getProperty (id::voicePressure);
            if (channelsData[index].voiceChannel == id::TIMBRE) x = voicesState.getChild (i).getProperty (id::voiceTimbre);
            a.add (curveValue (x,
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::curve),
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::handleOne),
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::handleTwo)));
        }
        return a;
    }
    juce::Array<float> getArrayD() const
    {
        juce::Array<float> a;
        int index = 3;
        for (int i = 0; i < voicesState.getNumChildren(); i++)
        {
            float x = 0;
            if (channelsData[index].voiceChannel == id::PRESSURE) x = voicesState.getChild (i).getProperty (id::voicePressure);
            if (channelsData[index].voiceChannel == id::TIMBRE) x = voicesState.getChild (i).getProperty (id::voiceTimbre);
            a.add (curveValue (x,
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::curve),
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::handleOne),
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::handleTwo)));
        }
        return a;
    }
    juce::Array<float> getArraySaturation() const
    {
        juce::Array<float> a;
        int index = 4;
        for (int i = 0; i < voicesState.getNumChildren(); i++)
        {
            float x = 0;
            if (channelsData[index].voiceChannel == id::PRESSURE) x = voicesState.getChild (i).getProperty (id::voicePressure);
            if (channelsData[index].voiceChannel == id::TIMBRE) x = voicesState.getChild (i).getProperty (id::voiceTimbre);
            auto cv = curveValue (x,
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::curve),
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::handleOne),
                               (float)routingBranch.getChildWithName (channelsData[index].voiceChannel).getChildWithName (channelsData[index].outputID).getProperty (id::handleTwo));
            auto value = valueTreeState.getParameter (channelsData[index].paramID)->convertFrom0to1 (cv);
            a.add (value);
        }
        return a;
    }
private:
    juce::ValueTree routingBranch;
    juce::ValueTree voicesState;
    juce::AudioProcessorValueTreeState& valueTreeState;

    struct ChannelData
    {
        ChannelData() = default;
        ChannelData (juce::String pid, 
                     juce::Identifier vc,
                     juce::Identifier oid,
                     bool controlled)
          : paramID (pid), 
            voiceChannel (vc),
            outputID (oid),
            isControlled (controlled)
        {}
        juce::String paramID;
        juce::Identifier voiceChannel;
        juce::Identifier outputID;
        bool isControlled;
    };
    juce::Array<ChannelData> channelsData
        {
            ChannelData ("TerrainModA", "null", "null", false),
            ChannelData ("TerrainModB", "null", "null", false),
            ChannelData ("TerrainModC", "null", "null", false),
            ChannelData ("TerrainModD", "null", "null", false),
            ChannelData ("TerrainSaturation", "null", "null", false)
        };

    void checkIfControlled()
    {
        for (auto& cd : channelsData) cd.isControlled = false;

        juce::Array<juce::Identifier> ids {id::OUTPUT_ONE, id::OUTPUT_TWO, id::OUTPUT_THREE,
                                           id::OUTPUT_FOUR, id::OUTPUT_FIVE, id::OUTPUT_SIX};
        routingBranch = valueTreeState.state.getChildWithName (id::PRESET_SETTINGS)
                                            .getChildWithName (id::MPE_ROUTING);

        auto pressureBranch = routingBranch.getChildWithName (id::PRESSURE);
        for (auto& id : ids)
        {
            for (auto& cd : channelsData)
            {
                if (pressureBranch.getChildWithName (id).getProperty (id::name).toString() == cd.paramID)
                {
                    cd.outputID = id;
                    cd.voiceChannel = (id::PRESSURE);
                    cd.isControlled = true;
                    break;
                } 
            }
        }
        auto timbreBranch = routingBranch.getChildWithName (id::TIMBRE);
        for (auto& id : ids)
        {
            for (auto& cd : channelsData)
            {
                if (timbreBranch.getChildWithName (id).getProperty (id::name).toString() == cd.paramID)
                {
                    cd.outputID = id;
                    cd.voiceChannel = (id::TIMBRE);
                    cd.isControlled = true;
                    break;
                } 
            }
        }
    }
    void valueTreeRedirected (juce::ValueTree& tree) override
    {
        juce::ignoreUnused (tree);
        checkIfControlled();
    }
    void valueTreePropertyChanged (juce::ValueTree& tree,
                                  const juce::Identifier& property) override
    {
        juce::ignoreUnused (tree);
        if (property == id::name) checkIfControlled();
    }
    static float curveValue (const float linearValue, const float curve, const float min, const float max)
    {
    
        float curvedValue = (float)std::pow (linearValue, 1.0f / curve);
        return juce::jmap (curvedValue, min, max);
    }
};
class Visualizer : public juce::Component, 
                   private juce::OpenGLRenderer, 
                   private juce::Timer
{
public:
    Visualizer (tp::WaveTerrainSynthesizerStandard& wts, 
                tp::WaveTerrainSynthesizerMPE& wtsmpe, 
                tp::Parameters parameters, 
                juce::ValueTree settings, 
                juce::ValueTree voicesStateBranch, 
                juce::AudioProcessorValueTreeState& apvts)
      : camera (mutex), 
        parameterWatcher (parameters), 
        mpeWatcher (voicesStateBranch, apvts),
        waveTerrainSynthesizerStandard (wts), 
        waveTerrainSynthesizerMPE (wtsmpe), 
        useMPE (settings, id::mpeEnabled, nullptr), 
        voicesState (voicesStateBranch)
    {
        mpeRouting = settings.getChildWithName (id::MPE_ROUTING);

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
    MPEWatcher mpeWatcher;
    std::unique_ptr<Trajectories> trajectories;
    tp::WaveTerrainSynthesizerStandard& waveTerrainSynthesizerStandard;
    tp::WaveTerrainSynthesizerMPE& waveTerrainSynthesizerMPE;

    juce::ValueTree settings;
    juce::ValueTree voicesState;
    juce::ValueTree mpeRouting;
    juce::CachedValue<bool> useMPE;

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
        if (!useMPE.get())
        {
            terrain->render(camera, color, ubo.index, ubo.a, ubo.b, ubo.c, ubo.d, ubo.saturation);
        }
        else
        {
            auto mpef = makeMPEFrame (mpeWatcher, parameterWatcher);
            if (mpef.isMPEControlled)
                terrain->renderMultiple (camera, color, ubo.index, mpef.a, mpef.b, mpef.c, mpef.d, mpef.saturation);
            else
                terrain->render(camera, color, ubo.index, ubo.a, ubo.b, ubo.c, ubo.d, ubo.saturation);
        }

        color = getLookAndFeel().findColour (juce::Slider::ColourIds::thumbColourId);
        trajectories->render (camera, color);
    }
    void openGLContextClosing() override 
    {
        terrain.reset();
        trajectories.reset();
    }
    struct MPEFrame
    {
        bool isMPEControlled;
        juce::Array<float> a;
        juce::Array<float> b;
        juce::Array<float> c;
        juce::Array<float> d;
        juce::Array<float> saturation;
    };
    
    static MPEFrame makeMPEFrame (const MPEWatcher& mpew, const ParameterWatcher& pw)
    {
        MPEFrame frame;
        if (mpew.aControlled() ||
            mpew.bControlled() ||
            mpew.cControlled() ||
            mpew.dControlled() ||
            mpew.saturationControlled())
        {
            frame.isMPEControlled = true;
        }
        else { frame.isMPEControlled = false; }
        
        auto ubo = pw.getUBO();

        frame.a.resize (15);
        frame.b.resize (15);
        frame.c.resize (15);
        frame.d.resize (15);
        frame.saturation.resize (15);
 
        if (!mpew.aControlled()) frame.a.fill (ubo.a);
        else frame.a = mpew.getArrayA();

        if (!mpew.bControlled()) frame.b.fill (ubo.b);
        else frame.b = mpew.getArrayB();

        if (!mpew.cControlled()) frame.c.fill (ubo.c);
        else frame.c = mpew.getArrayC();

        if (!mpew.dControlled()) frame.d.fill (ubo.d);
        else frame.d = mpew.getArrayD();

        if (!mpew.saturationControlled()) frame.saturation.fill (ubo.saturation);
        else frame.saturation = mpew.getArraySaturation();

        return frame;
    }
};

