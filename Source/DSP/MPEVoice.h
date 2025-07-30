#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <MTS-ESP/Client/libMTSClient.h>
#include "Trajectory.h"
#include "Terrain.h"
#include "../Utility/Identifiers.h"

#include "morphlib/Voice.h"
#include "morphlib/Synthesizer.h"

#include "MPEVoiceData.h"
namespace tp
{
class MPEVoice : public morph::Voice,
                 private juce::ValueTree::Listener
{
public:
    MPEVoice(Parameters& p, 
             juce::ValueTree SettingsBranch, 
             juce::ValueTree& MPESettings,
             MTSClient& mtsc, 
             juce::AudioProcessorValueTreeState& vts, 
             MPEVoiceData& vd)
      : terrain (p, vts, SettingsBranch.getChildWithName (id::MPE_ROUTING)),
        trajectory (terrain, p, SettingsBranch, mtsc, vts, vd), 
        routingBranch (SettingsBranch.getChildWithName (id::MPE_ROUTING)), 
        mpeSettingsBranch (MPESettings), 
        voiceData (vd),
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

    const float* getRawData() const { return trajectory.getRawData(); }
    void prepareToPlay (double newRate, int blockSize) override 
    { 
        terrain.prepareToPlay (newRate, blockSize);
        trajectory.prepareToPlay (newRate, blockSize); 
    }
    void setState (juce::ValueTree SettingsBranch) 
    { 
        terrain.setState (SettingsBranch.getChildWithName (id::MPE_ROUTING));
        trajectory.setState (SettingsBranch); 
        settingsBranch = SettingsBranch;
    } 
    bool isVoiceCurrentlyActive() const { return isActive(); }

    //Voice ===============================================
    void onNoteStart() override 
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

        voiceData.setVoiceActiveAT (true, note.midiChannel);
        voiceData.setPressureAT (note.pressure.asUnsignedFloat(), note.midiChannel);
        voiceData.setTimbreAT (note.timbre.asUnsignedFloat(), note.midiChannel);
    }
    void onNoteStop (bool allowTailOff) override
    {
        if (!allowTailOff) {
            auto note = getCurrentlyPlayingNote();
            voiceData.setPressureAT (0.0f, note.midiChannel);
            voiceData.setRMSAT (0.0f, note.midiChannel);
            voiceData.setVoiceActiveAT (false, note.midiChannel);
            clearCurrentNote();
        } 

        trajectory.stopNote(); 

        auto note = getCurrentlyPlayingNote();
        voiceData.setVoiceActiveAT (false, note.midiChannel);
        voiceData.setRMSAT( 0.0f, note.midiChannel );
    }
    
    void onNotePressureChanged() override 
    {
        auto note = getCurrentlyPlayingNote(); 
        pressure = note.pressure.asUnsignedFloat();
        terrain.setPressure (pressure);
        trajectory.setPressure (pressure);
        previousPressure = pressure;
        voiceData.setPressureAT( pressure, note.midiChannel);
    }
    void onNotePitchbendChanged() override {}
    void onPitchWheelChanged() override
    {
        trajectory.setFrequencySmooth (static_cast<float> (adjustedFrequency));
    }
    void setGlobalPitchWheel (float pitchWheelNormalized)
    {
        if (!pitchBendEnabled.get()) return;
        jassert (pitchWheelNormalized >= -1.0f && pitchWheelNormalized <= 1.0f);
        globalPitchBendSemitones = getGlobalPitchBendSemitones (pitchWheelNormalized);
        setPitchWheel (currentPitchWheel);
    }
    void onNoteTimbreChanged() override
    {
        auto note = getCurrentlyPlayingNote();
        timbre = note.timbre.asUnsignedFloat();
        terrain.setTimbre (timbre);
        trajectory.setTimbre (timbre);

        voiceData.setTimbreAT( timbre, note.midiChannel);
    }
    void onNoteKeyStateChanged() override {}
    void onAllocation(int maxBlockSize) override
    { 
        trajectory.allocate (maxBlockSize);
        terrain.allocate (maxBlockSize); 
    }
    void panic() override { onNoteStop (false); }
    
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

    void updateParameterBuffers() { terrain.updateParameterBuffers(); }
    float getPressure() { return pressure; }
    float getTimbre() { return timbre; }
    float getRMS() { return trajectory.getRMS(); }
private:
    MPETerrain    terrain;
    // juce::ValueTree voicesState;
    MPETrajectory trajectory;
    juce::ValueTree routingBranch;
    juce::ValueTree& mpeSettingsBranch;
    MPEVoiceData& voiceData;
    juce::ValueTree settingsBranch;
    // MTSClient& mtsClient;
    // float pressure {0.0f};
    // float timbre {0.0f};
    // float rms {0.0f};
    juce::CachedValue<float> releaseSensitivity;
    juce::CachedValue<bool> pitchBendEnabled;
    juce::CachedValue<int> divisionOfOctave;
    float previousPressure = 0.0f;
    // double initialNote;

    // float currentPitchWheel = 0.0f;
    // double globalPitchBendSemitones = 0.0f;
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
