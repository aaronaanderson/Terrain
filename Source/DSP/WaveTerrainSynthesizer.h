#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../Parameters.h"
#include "ADSR.h"

namespace tp {

class Terrain : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int midiNoteNumber) override { juce::ignoreUnused (midiNoteNumber); return true; }
    bool appliesToChannel (int midiChannel) override { juce::ignoreUnused (midiChannel); return true; }
};
class Trajectory : public juce::SynthesiserVoice
{
public:
    Trajectory()
    {
        envelope.prepare (sampleRate);
        envelope.setParameters ({200.0f, 20.0f, 0.7f, 1000.0f});
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

            o[i] += static_cast<float>(std::sin(phase)) * amplitude * static_cast<float> (envelope.calculateNext());
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
            // envelope.setParameters ({200.0f, 20.0f, 0.7f, 1000.0f});
            envelope.prepare (sampleRate);
            setFrequency (frequency);
        }
        juce::SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);
    }
private:
    ADSR envelope;
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
};
class WaveTerrainSynthesizer : public juce::Synthesiser, 
                               private juce::ValueTree::Listener
{
public:
    WaveTerrainSynthesizer (Parameters& p)
      : parameters (p)
    {
        setPolyphony (24);
        addSound (new Terrain());
    }


private:
    Parameters& parameters;

    void setPolyphony (int newPolyphony)
    {
        jassert (newPolyphony > 0);
        clearVoices();
        for (int i = 0; i < newPolyphony; i++)
            addVoice (new Trajectory ());
        
    }
};
}