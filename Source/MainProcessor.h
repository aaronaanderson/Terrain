#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <MTS-ESP/Client/libMTSClient.h>

#include "Parameters.h"
#include "Utility/Identifiers.h"
#include "Utility/PresetManager.h"
#include "Utility/Presets.h"

#include "DSP/WaveTerrainSynthesizerMPE.h"
#include "DSP/WaveTerrainSynthesizerStandard.h"
//==============================================================================
class MainProcessor  : public juce::AudioProcessor, 
                       private juce::ValueTree::Listener
{
public:
    //==============================================================================
    MainProcessor();
    ~MainProcessor() override;
    //==============================================================================
    void prepareToPlay (double sampleRate, int storedBufferSize) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    //==============================================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return valueTreeState; }
    juce::ValueTree& getState() { return valueTreeState.state; }
    juce::UndoManager& getUndoManager() { return undoManager; }
    PresetManager& getPresetManager() { return *presetManager.get(); }

    const tp::Parameters& getCastedParameters() const { return parameters; }
    tp::WaveTerrainSynthesizerStandard& getStandardWaveTerrainSynthesizer() { return *standardSynthesizer.get(); }
    tp::WaveTerrainSynthesizerMPE& getMPEWaveTerrainSynthesizer() { return *mpeSynthesizer.get(); }
    const juce::AudioProcessorValueTreeState& getAudioProcessorValueTreeState() {return valueTreeState; }
    bool getMTSConnectionStatus() { return MTS_HasMaster (mtsClient); }
    juce::String getTuningSystemName() { return MTS_GetScaleName (mtsClient); }
    juce::ValueTree& getMPESettings() { return mpeSettings; }
private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState valueTreeState;
    juce::UndoManager undoManager;
    tp::Parameters parameters;
    std::unique_ptr<PresetManager> presetManager;
    std::unique_ptr<tp::WaveTerrainSynthesizerStandard> standardSynthesizer;
    std::unique_ptr<tp::WaveTerrainSynthesizerMPE> mpeSynthesizer;
    juce::MPEInstrument instrument;
    std::atomic<bool> mpeOn;
    std::unique_ptr<juce::dsp::Oversampling<float>> overSampler;
    int storedFactor = -1; // initialize with invalid factor
    int storedBufferSize = 0;
    int maxSamplesPerBlock;
    double sampleRate;
    juce::AudioBuffer<float> renderBuffer;
    juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>, // DC Offset filter
                              juce::dsp::LadderFilter<float>, 
                              juce::dsp::Compressor<float>, 
                              juce::dsp::Gain<float>> outputChain;
    MTSClient* mtsClient = nullptr;
    void allocateMaxSamplesPerBlock (int maxSamples);
    void prepareOversampling (int bufferSize);
    juce::ValueTree verifiedSettings (juce::ValueTree);
    juce::ValueTree mpeSettings;
    void loadMPESettings();
    void saveMPESettings();
    void updateMPEParameters();
    void valueTreePropertyChanged (juce::ValueTree& tree, 
                                   const juce::Identifier& property) override;
    void valueTreeRedirected (juce::ValueTree& treeWhichHasBeenChanged) override;

    std::unique_ptr<juce::FileLogger> logger;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainProcessor)
};