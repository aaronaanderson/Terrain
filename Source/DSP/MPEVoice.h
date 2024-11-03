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
    MPEVoice(Terrain& t, 
             Parameters& p, 
             juce::ValueTree settingsBranch, 
             MTSClient& mtsc)
      :  trajectory (t, p, settingsBranch, mtsc)
    {}
    // Voice Interface ===================================================
    const float* getRawData() const override { return trajectory.getRawData(); }
    void prepareToPlay (double newRate, int blockSize) override { trajectory.prepareToPlay (newRate, blockSize); }
    void setState (juce::ValueTree settingsBranch) override { trajectory.setState (settingsBranch); } 
    bool isVoiceActive() const override { return isActive(); }
    // MPESynthesiser Voice ===============================================
    /** Called by the MPESynthesiser to let the voice know that a new note has started on it.
        This will be called during the rendering callback, so must be fast and thread-safe.
    */
    void noteStarted() override 
    {
        auto note = getCurrentlyPlayingNote();
        trajectory.startNote (note.initialNote, 
                              note.noteOnVelocity.asUnsignedFloat(), 
                              static_cast<float> (note.getFrequencyInHertz()));
        pressure.setCurrentAndTargetValue (note.pressure.asUnsignedFloat());
        timbre.setCurrentAndTargetValue (note.timbre.asUnsignedFloat());
    }

    /** Called by the MPESynthesiser to let the voice know that its currently playing note has stopped.
        This will be called during the rendering callback, so must be fast and thread-safe.

        If allowTailOff is false or the voice doesn't want to tail-off, then it must stop all
        sound immediately, and must call clearCurrentNote() to reset the state of this voice
        and allow the synth to reassign it another sound.

        If allowTailOff is true and the voice decides to do a tail-off, then it's allowed to
        begin fading out its sound, and it can stop playing until it's finished. As soon as it
        finishes playing (during the rendering callback), it must make sure that it calls
        clearCurrentNote().
    */
    void noteStopped (bool allowTailOff) override
    {
        if (!allowTailOff) clearCurrentNote();
        trajectory.stopNote();
    }

    /** Called by the MPESynthesiser to let the voice know that its currently playing note
        has changed its pressure value.
        This will be called during the rendering callback, so must be fast and thread-safe.
    */
    void notePressureChanged() override 
    {
        auto note = getCurrentlyPlayingNote();
        pressure.setTargetValue (note.pressure.asUnsignedFloat());
        trajectory.setAmplitude (note.pressure.asUnsignedFloat());
    }

    /** Called by the MPESynthesiser to let the voice know that its currently playing note
        has changed its pitchbend value.
        This will be called during the rendering callback, so must be fast and thread-safe.

        Note: You can call currentlyPlayingNote.getFrequencyInHertz() to find out the effective frequency
        of the note, as a sum of the initial note number, the per-note pitchbend and the master pitchbend.
    */
    void notePitchbendChanged() override
    {
        auto note = getCurrentlyPlayingNote();
        trajectory.setFrequencySmooth (static_cast<float> (note.getFrequencyInHertz()));
    }

    /** Called by the MPESynthesiser to let the voice know that its currently playing note
        has changed its timbre value.
        This will be called during the rendering callback, so must be fast and thread-safe.
    */
    void noteTimbreChanged() override
    {
        auto note = getCurrentlyPlayingNote();
        timbre.setTargetValue (note.timbre.asUnsignedFloat());
    }

    /** Called by the MPESynthesiser to let the voice know that its currently playing note
        has changed its key state.
        This typically happens when a sustain or sostenuto pedal is pressed or released (on
        an MPE channel relevant for this note), or if the note key is lifted while the sustained
        or sostenuto pedal is still held down.
        This will be called during the rendering callback, so must be fast and thread-safe.
    */
    void noteKeyStateChanged() override {}

    /** Renders the next block of data for this voice.

        The output audio data must be added to the current contents of the buffer provided.
        Only the region of the buffer between startSample and (startSample + numSamples)
        should be altered by this method.

        If the voice is currently silent, it should just return without doing anything.

        If the sound that the voice is playing finishes during the course of this rendered
        block, it must call clearCurrentNote(), to tell the synthesiser that it has finished.

        The size of the blocks that are rendered can change each time it is called, and may
        involve rendering as little as 1 sample at a time. In between rendering callbacks,
        the voice's methods will be called to tell it about note and controller events.
    */
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                          int startSample,
                          int numSamples) override
    {
        trajectory.renderNextBlock (outputBuffer, startSample, numSamples);
        if (trajectory.shouldClear()) clearCurrentNote();
    }
    void setCurrentSampleRate (double newRate) override { trajectory.setCurrentPlaybackSampleRate (newRate); }

private:
    Trajectory trajectory;
    juce::SmoothedValue<float> pressure;
    juce::SmoothedValue<float> timbre;
};
}