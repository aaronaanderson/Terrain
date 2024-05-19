#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../Parameters.h"
#include "ADSR.h"

namespace tp {
struct Point 
{  
    Point (float xp = 0.0f, float yp = 0.0f) : x(xp), y(yp) {}
    float x; float y; 

    Point operator+(const Point& other) { return Point(this->x + other.x, this->y + other.y); }
    Point operator*(float scalar) { return Point(this->x * scalar, this->y * scalar); }
};
struct ModSet
{
    ModSet (float ia = 0.0f, float ib = 0.0f, float ic = 0.0f, float id = 0.0f) : a(ia), b(ib), c(ic), d(id) {}
    float a, b, c, d;
};
struct InterpolatedParameter : private juce::AudioProcessorParameter::Listener
{
public:
    InterpolatedParameter (juce::AudioProcessorParameter* p)
      : parameter (p)
    {
        parameter->addListener (this);
    }
    ~InterpolatedParameter()
    {
        parameter->removeListener (this);
    }
    float getAt (int index) 
    {
        if (bufferSize == 0) return 0.0f; 
        return previousValue + ((index / static_cast<float> (bufferSize)) * (targetValue - previousValue));
    }
    void prepare (int blockSize) { bufferSize = blockSize; }

private:
    juce::AudioProcessorParameter* parameter;
    float previousValue = 0.0f;
    float targetValue = 0.0f;
    int bufferSize = 0;

    void parameterValueChanged (int parameterIndex, float newValue) override
    {
        juce::ignoreUnused (parameterIndex);
        previousValue = targetValue;
        targetValue = newValue;
    }

    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override { juce::ignoreUnused (parameterIndex, gestureIsStarting); }
};
class Terrain : public juce::SynthesiserSound
{
public:
    Terrain (Parameters& p)
      : parameters (p)
    {}
    bool appliesToNote (int midiNoteNumber) override { juce::ignoreUnused (midiNoteNumber); return true; }
    bool appliesToChannel (int midiChannel) override { juce::ignoreUnused (midiChannel); return true; }
    float sampleAt (Point p)
    {
        return (std::sin (p.x * 8), std::cos (p.y * 12));
    }
private:
    Parameters& parameters;
};
class Trajectory : public juce::SynthesiserVoice
{
public:
    Trajectory (Parameters& p)
      : parameters (p), 
        mod_a (parameters.trajectoryModA),
        mod_b (parameters.trajectoryModB),
        mod_c (parameters.trajectoryModC),
        mod_d (parameters.trajectoryModD)
    {
        envelope.prepare (sampleRate);
        envelope.setParameters ({200.0f, 20.0f, 0.7f, 1000.0f});

        functions = 
        {
            [&](float theta, ModSet m){ return Point(std::sin(theta) * m.a, std::cos(theta));} 
            ,[&](float theta, ModSet m)
                {   auto r = m.b + m.a * std::sin (theta);
                    return Point(r * std::cos(theta), r * std::sin(theta)); }
            ,[&](float theta, ModSet m)
                {   auto r = std::pow (juce::MathConstants<float>::euler, std::cos (theta + (m.a * juce::MathConstants<float>::twoPi))) - 2.0f * std::cos (4.0f * theta) + std::pow(std::sin((2.0f * theta - juce::MathConstants<float>::pi) / 24.0f), 5);
                    return Point(r * std::cos(phase), r * std::sin(theta)); }
            ,[&](float theta, ModSet m)
                {   auto r = (m.b * std::cos (2.0f * theta) - m.a * std::cos (theta));
                    return Point(r * std::cos(theta), r * std::sin(theta)); }
        };
    }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<Terrain*>(s) != nullptr; }
    void startNote (int midiNoteNumber,
                    float velocity,
                    juce::SynthesiserSound* sound,
                    int currentPitchWheelPosition) override 
    {
        juce::ignoreUnused (currentPitchWheelPosition, sound);
        
        setFrequency (static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber)));
        amplitude = velocity;
        terrain = dynamic_cast<Terrain*> (sound);
        envelope.noteOn();
    }
    void stopNote (float velocity, bool allowTailOff) override 
    { 
        juce::ignoreUnused (velocity, allowTailOff); 
        envelope.noteOff();
    }
    void pitchWheelMoved (int newPitchWheelValue) override { juce::ignoreUnused (newPitchWheelValue); }
    void controllerMoved (int controllerNumber, int newControllerValue) override { juce::ignoreUnused (controllerNumber, newControllerValue); }
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, 
                          int startSample, int numSamples) override 
    {
        auto* o = outputBuffer.getWritePointer(0);
        for(int i = startSample; i < startSample + numSamples; i++)
        {
            if(!envelope.isActive()) break;

            auto point = functions[*parameters.currentTrajectoryParameter](phase, getModSet (i)) * 0.2f;
            
            if (terrain != nullptr)
            {
                float outputSample = terrain->sampleAt (point);
                o[i] += outputSample * static_cast<float> (envelope.calculateNext()) * 0.1f;

            }
            phase = std::fmod (phase + phaseIncrement, juce::MathConstants<double>::twoPi);
            if(!envelope.isActive())
                clearCurrentNote();
        }
    } 
    void setCurrentPlaybackSampleRate (double newRate) override 
    {
        if (newRate != 0.0)
        {
            sampleRate = newRate;
            envelope.prepare (sampleRate);
            setFrequency (frequency);
        }
        juce::SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);
    }
    void prepareToPlay (double newRate, int blockSize)
    {
        juce::ignoreUnused (newRate);
        mod_a.prepare (blockSize);
        mod_b.prepare (blockSize);
        mod_c.prepare (blockSize);
        mod_d.prepare (blockSize);
    }
private:
    ADSR envelope;
    Parameters& parameters;
    Terrain* terrain;
    juce::Array<std::function<Point(float, ModSet)>> functions;
    InterpolatedParameter mod_a, mod_b, mod_c, mod_d;

    float frequency, amplitude;
    double phase = 0.0;
    double phaseIncrement;
    double sampleRate = 48000.0;

    void setFrequency (float newFrequency)
    {
        jassert (newFrequency > 0.0f);
        frequency = newFrequency;
        phaseIncrement = (frequency * juce::MathConstants<float>::twoPi) / sampleRate;
    }
    const ModSet getModSet (int sampleIndex)
    {
        return ModSet (mod_a.getAt (sampleIndex), mod_b.getAt (sampleIndex), 
                       mod_c.getAt (sampleIndex), mod_d.getAt (sampleIndex));
    }
};
class WaveTerrainSynthesizer : public juce::Synthesiser, 
                               private juce::ValueTree::Listener
{
public:
    WaveTerrainSynthesizer (Parameters& p)
      : parameters (p)
    {
        setPolyphony (24);
        addSound (new Terrain (parameters));
    }

    void prepareToPlay (double sr, int blockSize)
    {
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto v = getVoice (i);
            auto trajectory = dynamic_cast<Trajectory*> (v);
            trajectory->prepareToPlay (sr, blockSize);
        }
        setCurrentPlaybackSampleRate (sr);
    }

private:
    Parameters& parameters;

    void setPolyphony (int newPolyphony)
    {
        jassert (newPolyphony > 0);
        clearVoices();
        for (int i = 0; i < newPolyphony; i++)
            addVoice (new Trajectory (parameters));
    }
};
}