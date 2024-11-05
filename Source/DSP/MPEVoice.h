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
                 public juce::MPESynthesiserVoice, 
                 private juce::ValueTree::Listener
{
public:
    MPEVoice(Parameters& p, 
             juce::ValueTree settingsBranch, 
             juce::ValueTree& MPESettings,
             MTSClient& mtsc, 
             juce::AudioProcessorValueTreeState& vts)
      : terrain (p, vts, settingsBranch.getChildWithName (id::MPE_ROUTING)),
        trajectory (terrain, p, settingsBranch, mtsc, vts), 
        routingBranch (settingsBranch.getChildWithName (id::MPE_ROUTING)), 
        mpeSettingsBranch (MPESettings), 
        mtsClient (mtsc),
        pressureCurve (mpeSettingsBranch, id::pressureCurve, nullptr),
        timbreCurve (mpeSettingsBranch, id::timbreCurve, nullptr),
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
    ~MPEVoice() { mpeSettingsBranch.removeListener (this); }
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
        initialNote = note.initialNote;// MTS_NoteToFrequency (&mtsClient, static_cast<char> (note.initialNote), -1);
    }
    void noteStopped (bool allowTailOff) override
    {
        if (!allowTailOff) clearCurrentNote();
        trajectory.stopNote();
    }
    void notePressureChanged() override 
    {
        auto note = getCurrentlyPlayingNote();
        pressure = note.pressure.asUnsignedFloat();
        
        jassert (pressureCurve != 0.0f);
        float curvedPressure = static_cast<float> (std::pow(pressure, 1.0 / pressureCurve.get()));
        terrain.setPressure (curvedPressure);
        trajectory.setPressure (curvedPressure);
        
        if (curvedPressure <= 0.0f)
        {
            trajectory.setAmplitude (previousPressure);
            trajectory.setRelease();
        }
        else
        {
            trajectory.setAmplitude (curvedPressure);
            previousPressure = curvedPressure;
        }
    }
    void notePitchbendChanged() override
    {
        if (!pitchBendEnabled.get()) return;
        auto note = getCurrentlyPlayingNote();
        auto tunedBaseFrequency = MTS_NoteToFrequency (&mtsClient, static_cast<char> (initialNote), -1);
        auto adjustedFrequency = tunedBaseFrequency * semitonesToScalar (note.totalPitchbendInSemitones);
        trajectory.setFrequencySmooth (static_cast<float> (adjustedFrequency));
    }
    void noteTimbreChanged() override
    {
        auto note = getCurrentlyPlayingNote();
        timbre = note.timbre.asUnsignedFloat();
        float curvedTimbre = static_cast<float> (std::pow (timbre, 1.0 / timbreCurve.get()));
        terrain.setTimbre (curvedTimbre);
        trajectory.setTimbre (curvedTimbre);
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
    MPETrajectory trajectory;
    juce::ValueTree routingBranch;
    juce::ValueTree& mpeSettingsBranch;
    MTSClient& mtsClient;
    float pressure = 0.0f;
    float timbre = 0.0f;
    juce::CachedValue<float> pressureCurve;
    juce::CachedValue<float> timbreCurve;
    juce::CachedValue<float> releaseSensitivity;
    juce::CachedValue<bool> pitchBendEnabled;
    juce::CachedValue<int> divisionOfOctave;
    float previousPressure = 0.0f;
    double initialNote;
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
    double semitonesToScalar (double semitones) 
    { 
        double octaveDivision = static_cast<double> (divisionOfOctave.get());
        return std::pow (2, semitones / octaveDivision); 
    }
};
}