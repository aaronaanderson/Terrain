#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <MTS-ESP/Client/libMTSClient.h>
#include "VoiceInterface.h"
#include "Trajectory.h"
#include "Terrain.h"
#include "../Utility/Identifiers.h"

namespace tp
{
class MPEVoice : public VoiceInterface, 
                 public juce::MPESynthesiserVoice, 
                 private juce::ValueTree::Listener
{
public:
    MPEVoice(Parameters& p, 
             juce::ValueTree SettingsBranch, 
             juce::ValueTree& MPESettings,
             MTSClient& mtsc, 
             juce::AudioProcessorValueTreeState& vts, 
             juce::ValueTree voicesStateTree)
      : terrain (p, vts, SettingsBranch.getChildWithName (id::MPE_ROUTING)),
        voicesState (voicesStateTree),
        trajectory (terrain, p, SettingsBranch, mtsc, vts, voicesState), 
        routingBranch (SettingsBranch.getChildWithName (id::MPE_ROUTING)), 
        mpeSettingsBranch (MPESettings), 
        mtsClient (mtsc),
        releaseSensitivity (mpeSettingsBranch, id::releaseSensitivity, nullptr),
        pitchBendEnabled (mpeSettingsBranch, id::pitchBendEnabled, nullptr),
        divisionOfOctave (mpeSettingsBranch, id::pitchBendDivisionOfOctave, nullptr)
    {
        jassert (routingBranch.getType() == id::MPE_ROUTING);
        jassert (mpeSettingsBranch.getType() == id::MPE_SETTINGS);
        mpeSettingsBranch.addListener (this);

        terrain.setPressureSmoothing (mpeSettingsBranch.getProperty (id::pressureSmoothing));
        trajectory.setPressureSmoothing (mpeSettingsBranch.getProperty (id::pressureSmoothing));

        terrain.setTimbreSmoothing (mpeSettingsBranch.getProperty (id::timbreSmoothing));
        trajectory.setTimbreSmoothing (mpeSettingsBranch.getProperty (id::timbreSmoothing));
    }
    ~MPEVoice() override { /*mpeSettingsBranch.removeListener (this);*/ }
    // Voice Interface ===================================================
    const float* getRawData() const override { return trajectory.getRawData(); }
    void prepareToPlay (double newRate, int blockSize) override 
    { 
        terrain.prepareToPlay (newRate, blockSize);
        trajectory.prepareToPlay (newRate, blockSize); 
    }
    void setState (juce::ValueTree SettingsBranch) override 
    { 
        terrain.setState (SettingsBranch.getChildWithName (id::MPE_ROUTING));
        trajectory.setState (SettingsBranch); 
        settingsBranch = SettingsBranch;
    } 
    bool isVoiceCurrentlyActive() const override { return isActive(); }
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
                              note.timbre.asUnsignedFloat(), 
                              note.midiChannel);
        initialNote = note.initialNote;// MTS_NoteToFrequency (&mtsClient, static_cast<char> (note.initialNote), -1);

        juce::MessageManager::callAsync([this, note]() 
            {
                auto channelState = voicesState.getChild (static_cast<int> (note.midiChannel - 2));
                channelState.setProperty (id::voiceActive, true, nullptr);
                channelState.setProperty (id::voicePressure, note.pressure.asUnsignedFloat(), nullptr);
                channelState.setProperty (id::voiceTimbre, note.timbre.asUnsignedFloat(), nullptr);
            });
    }
    void noteStopped (bool allowTailOff) override
    {
        if (!allowTailOff) clearCurrentNote();
        trajectory.stopNote(); 

        auto note = getCurrentlyPlayingNote();
        auto channelState = voicesState.getChild (static_cast<int> (juce::jlimit (0, 15, note.midiChannel - 2)));

        juce::MessageManager::callAsync([this, note]() 
            {
                auto channelState = voicesState.getChild (static_cast<int> (note.midiChannel - 2));
                if (!channelState.isValid()) return;
                channelState.setProperty (id::voiceRMS, 0.0f, nullptr);
                channelState.setProperty (id::voiceActive, false, nullptr);
            });
    }
    
    void notePressureChanged() override 
    {
        auto note = getCurrentlyPlayingNote();
        pressure = note.pressure.asUnsignedFloat();
        terrain.setPressure (pressure);
        trajectory.setPressure (pressure);
        
        if (pressure <= 0.0f)
        {
            // trajectory.setAmplitude (previousPressure);
            trajectory.setRelease();
        }
        else
        {
            // trajectory.setAmplitude (pressure);
            previousPressure = pressure;
        }

        juce::MessageManager::callAsync([this, note]() 
            {
                auto channelState = voicesState.getChild (static_cast<int> (note.midiChannel - 2));
                channelState.setProperty (id::voicePressure, pressure, nullptr);
            });
    }
    void notePitchbendChanged() override {}
    void setPitchWheel (float pitchWheel)
    {
        if (!pitchBendEnabled.get()) return;
        jassert (pitchWheel >= -1.0f && pitchWheel <= 1.0f);
        currentPitchWheel = pitchWheel;
        auto tunedBaseFrequency = MTS_NoteToFrequency (&mtsClient, static_cast<char> (initialNote), -1);
        auto semitones = getPitchBendToSemitones (pitchWheel);
        auto adjustedFrequency = tunedBaseFrequency * semitonesToScalar (semitones + globalPitchBendSemitones);
        trajectory.setFrequencySmooth (static_cast<float> (adjustedFrequency));
    }
    void setGlobalPitchWheel (float pitchWheelNormalized)
    {
        if (!pitchBendEnabled.get()) return;
        jassert (pitchWheelNormalized >= -1.0f && pitchWheelNormalized <= 1.0f);
        globalPitchBendSemitones = getGlobalPitchBendSemitones (pitchWheelNormalized);
        setPitchWheel (currentPitchWheel);
    }
    void noteTimbreChanged() override
    {
        auto note = getCurrentlyPlayingNote();
        timbre = note.timbre.asUnsignedFloat();
        terrain.setTimbre (timbre);
        trajectory.setTimbre (timbre);

        juce::MessageManager::callAsync([this, note]() 
            {
                auto channelState = voicesState.getChild (static_cast<int> (note.midiChannel - 2));
                channelState.setProperty (id::voiceTimbre, timbre, nullptr);
            });
    }
    void noteKeyStateChanged() override {}
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                          int startSample,
                          int numSamples) override
    {
        trajectory.renderNextBlock (outputBuffer, startSample, numSamples);
        if (trajectory.shouldClear())
        {
            
            clearCurrentNote();
        }
    }
    void setCurrentSampleRate (double newRate) override { trajectory.setCurrentPlaybackSampleRate (newRate); }
    void allocate (int maxBlockSize) 
    { 
        trajectory.allocate (maxBlockSize);
        terrain.allocate (maxBlockSize); 
    }
    void updateParameterBuffers() { terrain.updateParameterBuffers(); }
    float getPressure() { return pressure; }
    float getTimbre() { return timbre; }
private:
    MPETerrain    terrain;
    juce::ValueTree voicesState;
    MPETrajectory trajectory;
    juce::ValueTree routingBranch;
    juce::ValueTree& mpeSettingsBranch;
    juce::ValueTree settingsBranch;
    MTSClient& mtsClient;
    float pressure = 0.0f;
    float timbre = 0.0f;

    juce::CachedValue<float> releaseSensitivity;
    juce::CachedValue<bool> pitchBendEnabled;
    juce::CachedValue<int> divisionOfOctave;
    float previousPressure = 0.0f;
    double initialNote;

    float currentPitchWheel = 0.0f;
    double globalPitchBendSemitones = 0.0f;
    void valueTreePropertyChanged (juce::ValueTree& tree,
                                   const juce::Identifier& property) override
    {
        juce::ignoreUnused (tree);
        if (property == id::pressureSmoothing)
        {
            terrain.setPressureSmoothing (tree.getProperty (property));
            trajectory.setPressureSmoothing (tree.getProperty (property));
        }
        else if (property == id::timbreSmoothing)
        {
            terrain.setTimbreSmoothing (tree.getProperty (property));
            trajectory.setTimbreSmoothing (tree.getProperty (property));
        }
    }
    double getPitchBendToSemitones (float pitchWheelNormalized)
    {
        return juce::jmap ((double)pitchWheelNormalized, -1.0, 1.0, -48.0, 48.0);
    }
    double getGlobalPitchBendSemitones (float pitchWheelNormalized)
    {
        double bend = settingsBranch.getProperty (id::pitchBendRange);
        return juce::jmap ((double) pitchWheelNormalized, -1.0, 1.0, -bend, bend);
    }
    double semitonesToScalar (double semitones) 
    { 
        double octaveDivision = static_cast<double> (divisionOfOctave.get());
        return std::pow (2, semitones / octaveDivision); 
    }
};
}
