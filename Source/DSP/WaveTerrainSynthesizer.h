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
struct SmoothedParameter : private juce::AudioProcessorParameter::Listener
{
public:
    SmoothedParameter (juce::RangedAudioParameter* p)
      : rangedParameter (p), 
        smoothedValue (p->convertFrom0to1 (p->getValue()))
    {
        rangedParameter->addListener (this);
    }
    ~SmoothedParameter()
    {
        rangedParameter->removeListener (this);
    }
    void noteOn() 
    {
        smoothedValue.setCurrentAndTargetValue (rangedParameter->convertFrom0to1 (rangedParameter->getValue())); 
    }
    float getNext() 
    {
        return smoothedValue.getNextValue();
    }
    void prepare (double sampleRate) 
    { 
        smoothedValue.reset (sampleRate, 0.02f);
    }

private:
    juce::RangedAudioParameter* rangedParameter;
    juce::SmoothedValue<float> smoothedValue;
    float previousValue = 0.0f;
    float targetValue = 0.0f;
    int bufferSize = 0;

    void parameterValueChanged (int parameterIndex, float newValue) override
    {
        juce::ignoreUnused (parameterIndex);
        smoothedValue.setTargetValue (rangedParameter->convertFrom0to1 (newValue));
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
        return (std::sin (p.x * 8.0f) * std::cos (p.y * 12.0f));
    }
private:
    Parameters& parameters;
};
class Trajectory : public juce::SynthesiserVoice
{
public:
    Trajectory (Parameters& p)
      : voiceParameters (p)
    {
        envelope.prepare (sampleRate);
        envelope.setParameters ({200.0f, 20.0f, 0.7f, 1000.0f});

        functions = 
        {
            [&](float theta, ModSet m){ return Point(std::sin(theta) * m.a, std::cos(theta));} 
            ,[&](float theta, ModSet m)
                {   float r = m.b + m.a * std::sin (theta);
                    return Point(r * std::cos(theta), r * std::sin(theta)); }
            ,[&](float theta, ModSet m)
                {   float r = std::pow (juce::MathConstants<float>::euler, std::cos (theta + (m.a * juce::MathConstants<float>::twoPi))) - 2.0f * std::cos (4.0f * theta) + std::pow(std::sin((2.0f * theta - juce::MathConstants<float>::pi) / 24.0f), 5);
                    return Point(r * std::cos(phase), r * std::sin(theta)); }
            ,[&](float theta, ModSet m)
                {   float r = (m.b * std::cos (2.0f * theta) - m.a * std::cos (theta));
                    return Point(r * std::cos(theta), r * std::sin(theta)); }
        };
    }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<Terrain*>(s) != nullptr; }
    void startNote (int midiNoteNumber,
                    float velocity,
                    juce::SynthesiserSound* sound,
                    int currentPitchWheelPosition) override 
    {
        juce::ignoreUnused (currentPitchWheelPosition);
        
        setFrequency (static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber)));
        amplitude = velocity;
        terrain = dynamic_cast<Terrain*> (sound);
        envelope.noteOn();
        voiceParameters.noteOn();
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

            auto point = functions[*voiceParameters.currentTrajectory](static_cast<float> (phase), getModSet());
            
            point = point * (voiceParameters.size.getNext());
            point = rotate (point, voiceParameters.rotation.getNext());
            point = translate (point, voiceParameters.translationX.getNext(), voiceParameters.translationY.getNext());

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
        juce::ignoreUnused (blockSize);
        voiceParameters.resetSampleRate (newRate);
    }
private:
    ADSR envelope;
    Terrain* terrain;
    juce::Array<std::function<Point(float, ModSet)>> functions;
    struct VoiceParameters
    {
        VoiceParameters (Parameters& p)
          : currentTrajectory (p.currentTrajectory),
            mod_a (p.trajectoryModA),
            mod_b (p.trajectoryModB),
            mod_c (p.trajectoryModC),
            mod_d (p.trajectoryModD), 
            size (p.trajectorySize), 
            rotation (p.trajectoryRotation), 
            translationX (p.trajectoryTranslationX), 
            translationY (p.trajectoryTranslationY)
        {}

        void noteOn()
        {
            mod_a.noteOn();
            mod_b.noteOn();
            mod_c.noteOn();
            mod_d.noteOn();
            size.noteOn();
            rotation.noteOn();
            translationX.noteOn();
            translationY.noteOn();
        }

        void resetSampleRate(double newSampleRate)
        {
            mod_a.prepare (newSampleRate);
            mod_b.prepare (newSampleRate);
            mod_c.prepare (newSampleRate);
            mod_d.prepare (newSampleRate);
            size.prepare (newSampleRate);
            rotation.prepare (newSampleRate);
            translationX.prepare (newSampleRate); 
            translationY.prepare (newSampleRate);
        }

        SmoothedParameter mod_a, mod_b, mod_c, mod_d;
        SmoothedParameter size, rotation, translationX, translationY;
        tp::ChoiceParameter* currentTrajectory;
    };
    VoiceParameters voiceParameters;

    float frequency = 440.0f;
    float amplitude = 1.0;
    double phase = 0.0;
    double phaseIncrement;
    double sampleRate = 48000.0;

    void setFrequency (float newFrequency)
    {
        jassert (newFrequency > 0.0f);
        frequency = newFrequency;
        phaseIncrement = (frequency * juce::MathConstants<float>::twoPi) / sampleRate;
    }
    Point rotate (const Point p, const float theta)
    {
        Point newPoint ((p.x * std::cos (theta)) - (p.y * std::sin (theta)), 
                        (p.y * std::cos (theta)) + (p.x * std::sin (theta)));
        return newPoint;
    }
    Point translate (const Point p, float x, float y)
    {
        Point newPoint (p.x + x, p.y + y);
        return newPoint;
    }
    const ModSet getModSet()
    {
        return ModSet (voiceParameters.mod_a.getNext(), voiceParameters.mod_b.getNext(), 
                       voiceParameters.mod_c.getNext(), voiceParameters.mod_d.getNext());
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
            if (trajectory != nullptr)
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