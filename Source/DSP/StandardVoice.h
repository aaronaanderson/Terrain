#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <MTS-ESP/Client/libMTSClient.h>
#include "VoiceInterface.h"
#include "Trajectory.h"
#include "Terrain.h"

namespace tp
{
class StandardVoice : public VoiceInterface, 
                      public juce::SynthesiserVoice
{
public:
    StandardVoice (Terrain& t, 
                   Parameters& p, 
                   juce::ValueTree settingsBranch, 
                   MTSClient& mtsc)
      : trajectory (t, p, settingsBranch, mtsc)
    {}
    // Voice Interface ===================================================
    const float* getRawData() const override { return trajectory.getRawData(); }
    void prepareToPlay (double newRate, int blockSize) override { trajectory.prepareToPlay (newRate, blockSize); }
    void setState (juce::ValueTree settingsBranch) override { trajectory.setState (settingsBranch); }
    // Synthesiser Voice ==================================================
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<DummySound*>(s) != nullptr; }
    void startNote (int midiNoteNumber,
                    float velocity,
                    juce::SynthesiserSound* /*sound*/,
                    int currentPitchWheelPosition) override 
    {   
        trajectory.startNote (midiNoteNumber, velocity, currentPitchWheelPosition);
    }
    void stopNote (float /*velocity*/, bool allowTailOff) override 
    { 
        if (!allowTailOff) clearCurrentNote(); 
        trajectory.stopNote();
    }
    void pitchWheelMoved (int newPitchWheelValue) override 
    { 
        trajectory.pitchWheelMoved (newPitchWheelValue);
    }
    void controllerMoved (int controllerNumber, int newControllerValue) override 
    { 
        juce::ignoreUnused (controllerNumber, newControllerValue); 
        trajectory.controllerMoved();
    }
    void renderNextBlock (juce::AudioBuffer<double>& ob, int ss, int nums) override 
    {  
        juce::ignoreUnused (ob, ss, nums);
        // trajectory.renderNextBlock (ob, ss, nums);
        // if (trajectory.shouldClear()) clearCurrentNote();
    }
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, 
                          int startSample, int numSamples) override 
    {
        trajectory.renderNextBlock (outputBuffer, startSample, numSamples);
        if (trajectory.shouldClear()) clearCurrentNote();
    }
    void setCurrentPlaybackSampleRate (double newRate) override 
    {
        trajectory.setCurrentPlaybackSampleRate (newRate);
    }
    bool isVoiceActive() const { return juce::SynthesiserVoice::isVoiceActive(); }
private:
    StandardTrajectory trajectory;
};
}