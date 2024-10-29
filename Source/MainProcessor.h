#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include "Identifiers.h"
#include "DSP/WaveTerrainSynthesizer.h"
#include "Utility/PresetManager.h"

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
    tp::WaveTerrainSynthesizer& getWaveTerrainSynthesizer() { return *synthesizer.get(); }
private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState valueTreeState;
    juce::UndoManager undoManager;
    tp::Parameters parameters;
    std::unique_ptr<PresetManager> presetManager;
    std::unique_ptr<tp::WaveTerrainSynthesizer> synthesizer;
    std::unique_ptr<juce::dsp::Oversampling<float>> overSampler;
    int storedFactor = -1; // initialize with invalid factor
    juce::AudioBuffer<float> renderBuffer;

    juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>, // DC Offset filter
                              juce::dsp::LadderFilter<float>, 
                              juce::dsp::Compressor<float>, 
                              juce::dsp::Gain<float>> outputChain;

    int storedBufferSize = 0;
    int maxSamplesPerBlock;
    double sampleRate;

    void allocateMaxSamplesPerBlock (int maxSamples);
    void prepareOversampling (int bufferSize);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainProcessor)
};