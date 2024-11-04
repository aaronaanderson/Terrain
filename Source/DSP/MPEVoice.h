#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <MTS-ESP/Client/libMTSClient.h>
#include "VoiceInterface.h"
#include "Trajectory.h"
#include "Terrain.h"

namespace tp
{
class MPEVoice : public VoiceInterface, 
                 public juce::MPESynthesiserVoice
{
public:
    MPEVoice(Parameters& p, 
             juce::ValueTree settingsBranch, 
             MTSClient& mtsc, 
             juce::AudioProcessorValueTreeState& vts)
      : terrain (p, vts, settingsBranch.getChildWithName (id::MPE_ROUTING)),
        trajectory (terrain, p, settingsBranch, mtsc, vts), 
        routingBranch (settingsBranch.getChildWithName (id::MPE_ROUTING))
    {
        jassert (routingBranch.getType() == id::MPE_ROUTING);
    }
    // Voice Interface ===================================================
    const float* getRawData() const override { return trajectory.getRawData(); }
    void prepareToPlay (double newRate, int blockSize) override 
    { 
        terrain.prepareToPlay (newRate, blockSize);
        trajectory.prepareToPlay (newRate, blockSize); 
    }
    void setState (juce::ValueTree settingsBranch) override 
    { 
        terrain.setState (settingsBranch.getChildWithName (id::MPE_ROUTING));
        trajectory.setState (settingsBranch); 
    } 
    bool isVoiceActive() const override { return isActive(); }
    // MPESynthesiser Voice ===============================================
    void noteStarted() override 
    {
        auto note = getCurrentlyPlayingNote();
        terrain.noteOn (note.pressure.asUnsignedFloat(), 
                        note.timbre.asUnsignedFloat());
        trajectory.startNote (note.initialNote, 
                              note.noteOnVelocity.asUnsignedFloat(), 
                              static_cast<float> (note.getFrequencyInHertz()), 
                              note.pressure.asUnsignedFloat(), 
                              note.timbre.asUnsignedFloat());
    }
    void noteStopped (bool allowTailOff) override
    {
        if (!allowTailOff) clearCurrentNote();
        trajectory.stopNote();
    }
    void notePressureChanged() override 
    {
        auto note = getCurrentlyPlayingNote();
        terrain.setPressure (note.pressure.asUnsignedFloat());
        trajectory.setPressure (note.pressure.asUnsignedFloat());
        trajectory.setAmplitude (note.pressure.asUnsignedFloat());
    }

    void notePitchbendChanged() override
    {
        auto note = getCurrentlyPlayingNote();
        trajectory.setFrequencySmooth (static_cast<float> (note.getFrequencyInHertz()));
    }
    void noteTimbreChanged() override
    {
        auto note = getCurrentlyPlayingNote();
        terrain.setTimbre (note.timbre.asUnsignedFloat());
        trajectory.setTimbre (note.timbre.asUnsignedFloat());
    }
    void noteKeyStateChanged() override {}
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                          int startSample,
                          int numSamples) override
    {
        trajectory.renderNextBlock (outputBuffer, startSample, numSamples);
        if (trajectory.shouldClear()) clearCurrentNote();
    }
    void setCurrentSampleRate (double newRate) override { trajectory.setCurrentPlaybackSampleRate (newRate); }
    void allocate (int maxBlockSize) { terrain.allocate (maxBlockSize); }
    void updateParameterBuffers() { terrain.updateParameterBuffers(); }
private:
    MPETerrain    terrain;
    MPETrajectory trajectory;
    juce::ValueTree routingBranch;
    juce::SmoothedValue<float> pressure;
    juce::SmoothedValue<float> timbre;
};
}