#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "DataTypes.h"
#include "../Parameters.h"

namespace  tp {
// Juce::Synthesiser requires a SynthesiserSound, 
// but we will not be using this.
struct DummySound : public juce::SynthesiserSound
{
    bool appliesToNote (int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel (int /*midiChannel*/) override { return true; }
};
// Instead each voice will have a reference to a
// Terrain. This helps making it compatible with
// juce::MPESynthesiser since it doesn't make use
// of the Sound paradigm 
class Terrain
{
public:
    Terrain (Parameters& p)
      : parameters (p)
    {}
    virtual void prepareToPlay(double sampleRate, int blockSize) = 0;
    virtual void allocate (int maxNumSamples) = 0;
    virtual void updateParameterBuffers() = 0;

    float sampleAt (Point p, int bufferIndex)
    {
        auto m = getModSet (bufferIndex);
        float output = 0.0f;
        switch (*parameters.currentTerrain)
        {
            case 0:
                output = std::sin(p.x * 6.0f * (m.a + 0.5f)) * std::sin(p.y * 6.0f * (m.b + 0.5f));
                break;
            case 1:
                output = std::sin((p.x * juce::MathConstants<float>::twoPi) * (p.x * 3.0f * m.a) + (m.b * juce::MathConstants<float>::twoPi)) * 
                       std::sin((p.y * juce::MathConstants<float>::twoPi) * (p.y * 3.0f * m.a) + (m.b * -juce::MathConstants<float>::twoPi));
                break;
            case 2:
                output = std::cos(dfc (p) * juce::MathConstants<float>::twoPi * (m.a * 5.0f + 1.0f) + (m.b * juce::MathConstants<float>::twoPi));
                break;
            case 3:
                output = (1.0f - (p.x * p.y)) * std::cos((m.a * 14.0f + 1.0f) * (1.0f - p.x * p.y));
                break;
            case 4:
            {
                float c = m.a * 0.5f + 0.25f;
                float d = m.b * 16.0f + 4.0f;  
                output = c * p.x * std::cos((1.0f - c) * d * juce::MathConstants<float>::pi * p.x * p.y)  +  (1.0f - c) * p.y * cos(c * d * juce::MathConstants<float>::pi * p.x * p.y);
            }
                break;
            case 5:
            {
                float aa = m.a * 4.0f + 1.0f;
                float bb = m.b * 4.0f + 1.0f;
                float cc = m.c * 0.8f + 0.1f;
                output = ((std::pow (aa * p.x, 2.0f) + std::pow (bb * p.y, 2.0f)) * 
                           std::pow (cc, (std::pow (4.0f * p.x, 2.0f) + 
                                          std::pow (4.0f * p.y, 2.0f)))) * 2.0f - 1.0f;
            }
                break;
            case 6: // system 12
            {
                float aa = m.a * 4.0f + 1.0f;
                float bb = m.b * 4.0f + 1.0f;
                output = std::sin(std::pow (aa * p.x, 2.0f) + std::pow (bb * p.y, 2.0f));
            }
                break;
            case 7: // system 14
            {
                float aa = m.a * 36.0f + 6.0f;
                float bb = m.b * 2.0f - 1.0f;
                float cc = m.c * 2.0f - 1.0f;
                output = std::cos (aa * std::sin (std::sqrt (std::pow (p.x + bb, 2.0f) + std::pow (p.y + cc, 2.0f))));
            }
                break;
            case 8: // system 15
            {
                float aa = m.a * 36.0f;
                output = std::cos (( aa * std::sin (std::sqrt (std::pow (p.x + 1.1f, 2.0f) + std::pow (p.y + 1.1f, 2.0f)))) - (4.0f * std::atan ((p.y + 1.1f) / (p.x + 1.1f))));
            }
                break;
            default:
            jassertfalse;
        }

        return saturate (output, bufferIndex);
    }
protected:
    Parameters& parameters;
    virtual const ModSet getModSet (int index) = 0;
    // distance from center
    inline float dfc (Point p) { return std::sqrt (p.x * p.x + p.y * p.y); }
    virtual float saturate (float signal, int index) = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Terrain)
};
class StandardTerrain : public Terrain
{
public:
    StandardTerrain (Parameters& p)
      : Terrain (p),
        modA (p.terrainModA), 
        modB (p.terrainModB), 
        modC (p.terrainModC), 
        modD (p.terrainModD), 
        saturation (p.terrainSaturation)
    {}
    void prepareToPlay(double sampleRate, int blockSize) override
    {
        modA.prepareToPlay (sampleRate, blockSize);
        modB.prepareToPlay (sampleRate, blockSize);
        modC.prepareToPlay (sampleRate, blockSize);
        modD.prepareToPlay (sampleRate, blockSize);
        saturation.prepareToPlay (sampleRate, blockSize);
    }
    void allocate (int maxNumSamples) override
    {
        modA.allocate (maxNumSamples);
        modB.allocate (maxNumSamples);
        modC.allocate (maxNumSamples);
        modD.allocate (maxNumSamples);
        saturation.allocate (maxNumSamples);
    }
    void updateParameterBuffers() override
    {
        modA.updateBuffer();
        modB.updateBuffer();
        modC.updateBuffer();
        modD.updateBuffer();
        saturation.updateBuffer();
    }
private:
    BufferedSmoothParameter modA, modB, modC, modD, saturation;

    const ModSet getModSet (int index) override
    {
        return ModSet (modA.getAt (index), modB.getAt (index), 
                       modC.getAt (index), modD.getAt (index));
    }
    float saturate (float signal, int bufferIndex) override
    {
        auto scale = saturation.getAt (bufferIndex);
        return juce::dsp::FastMathApproximations::tanh<float> (signal * scale * 1.31303528551f);
    }   
    // distance from center
    inline float dfc (Point p) { return std::sqrt (p.x * p.x + p.y * p.y); }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandardTerrain)
};

class MPETerrain : public Terrain
{
public:
    MPETerrain (Parameters& p,
                juce::AudioProcessorValueTreeState& apvts, 
                juce::ValueTree mpeRoutingBranch)
      : Terrain (p),
        modA (p.terrainModA, apvts, mpeRoutingBranch), 
        modB (p.terrainModB, apvts, mpeRoutingBranch), 
        modC (p.terrainModC, apvts, mpeRoutingBranch), 
        modD (p.terrainModD, apvts, mpeRoutingBranch), 
        saturation (p.terrainSaturation, apvts, mpeRoutingBranch)
    {}
    void prepareToPlay(double sampleRate, int blockSize) override
    {
        modA.prepareToPlay (sampleRate, blockSize);
        modB.prepareToPlay (sampleRate, blockSize);
        modC.prepareToPlay (sampleRate, blockSize);
        modD.prepareToPlay (sampleRate, blockSize);
        saturation.prepareToPlay (sampleRate, blockSize);
    }
    void allocate (int maxNumSamples) override
    {
        modA.allocate (maxNumSamples);
        modB.allocate (maxNumSamples);
        modC.allocate (maxNumSamples);
        modD.allocate (maxNumSamples);
        saturation.allocate (maxNumSamples);
    }
    void updateParameterBuffers() override
    {
        modA.updateBuffer();
        modB.updateBuffer();
        modC.updateBuffer();
        modD.updateBuffer();
        saturation.updateBuffer();
    }
    void noteOn (float mpePressure, float mpeTimbre)
    {
        modA.noteOn (mpePressure, mpeTimbre);
        modB.noteOn (mpePressure, mpeTimbre);
        modC.noteOn (mpePressure, mpeTimbre);
        modD.noteOn (mpePressure, mpeTimbre);
        saturation.noteOn (mpePressure, mpeTimbre);
    }
    void setTimbre (float newTimbre)
    {
        modA.setTimbre (newTimbre);
        modB.setTimbre (newTimbre);
        modC.setTimbre (newTimbre);
        modD.setTimbre (newTimbre);
        saturation.setTimbre (newTimbre);
    }
    void setPressure (float newPressure)
    {
        modA.setPressure (newPressure);
        modB.setPressure (newPressure);
        modC.setPressure (newPressure);
        modD.setPressure (newPressure);
        saturation.setPressure (newPressure);
    }
    void setState (juce::ValueTree mpeRoutingBranch) 
    { 
        modA.setState (mpeRoutingBranch);
        modB.setState (mpeRoutingBranch);
        modC.setState (mpeRoutingBranch);
        modD.setState (mpeRoutingBranch);
        saturation.setState (mpeRoutingBranch);
    }
    void setPressureSmoothing (float pressureSmoothing)
    {
        modA.setPressureSmoothing (pressureSmoothing);
        modB.setPressureSmoothing (pressureSmoothing);
        modC.setPressureSmoothing (pressureSmoothing);
        modD.setPressureSmoothing (pressureSmoothing);
        saturation.setPressureSmoothing (pressureSmoothing);
    }
    void setTimbreSmoothing (float timbreSmoothing)
    {
        modA.setTimbreSmoothing (timbreSmoothing);
        modB.setTimbreSmoothing (timbreSmoothing);
        modC.setTimbreSmoothing (timbreSmoothing);
        modD.setTimbreSmoothing (timbreSmoothing);
        saturation.setTimbreSmoothing (timbreSmoothing);
    }
private:
    BufferedMPESmoothParameter modA, modB, modC, modD, saturation;
    const ModSet getModSet (int index) override
    {
        return ModSet (modA.getAt (index), modB.getAt (index), 
                       modC.getAt (index), modD.getAt (index));
    }
    float saturate (float signal, int bufferIndex) override
    {
        auto scale = saturation.getAt (bufferIndex);
        return juce::dsp::FastMathApproximations::tanh<float> (signal * scale * 1.31303528551f);
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPETerrain)
};
} // end namespace tp