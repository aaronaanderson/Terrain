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
    PresetManager& getPresetManager() { return presetManager; }

    const tp::Parameters& getCastedParameters() const { return parameters; }
    tp::WaveTerrainSynthesizer& getWaveTerrainSynthesizer() { return *synthesizer.get(); }
private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState valueTreeState;
    juce::UndoManager undoManager;
    tp::Parameters parameters;
    PresetManager presetManager;
    std::unique_ptr<tp::WaveTerrainSynthesizer> synthesizer;
    std::unique_ptr<juce::dsp::Oversampling<float>> overSampler;
    int storedFactor = -1; // initialize with invalid factor
    juce::AudioBuffer<float> renderBuffer;

    // juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>> outputChain;
    juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>, // DC Offset filter
                              juce::dsp::LadderFilter<float>, 
                              juce::dsp::Compressor<float>, 
                              juce::dsp::Gain<float>> outputChain;

    int storedBufferSize = 0;
    int maxSamplesPerBlock;
    double sampleRate;

    // const juce::String trajectoryNameFromIndex (int i);

    void allocateMaxSamplesPerBlock (int maxSamples)
    {
        auto settingsTree = valueTreeState.state.getChildWithName (id::PRESET_SETTINGS);
        auto overSamplingFactor = static_cast<int> (settingsTree.getProperty (id::oversampling));

        synthesizer->allocate (maxSamples * static_cast<int> (std::pow (2, overSamplingFactor)));
        overSampler = std::make_unique<juce::dsp::Oversampling<float>> (1, 
                                                                        overSamplingFactor, 
                                                                        juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR);
        overSampler->initProcessing (static_cast<size_t> (maxSamples));
    }
    void prepareOversampling (int bufferSize)
    {
        auto presetsTree = valueTreeState.state.getChildWithName (id::PRESET_SETTINGS);
        auto overSamplingFactor = static_cast<int> (presetsTree.getProperty (id::oversampling));
        
        if (overSamplingFactor == storedFactor && bufferSize == storedBufferSize) return;
        //=============Very bad to allocate here, threaded solution instead of blocking should come in the future
        //===The situation only arises if oversampling factor has changed
        if (overSamplingFactor != storedFactor)
        {
            synthesizer->allocate (maxSamplesPerBlock * static_cast<int> (std::pow (2, overSamplingFactor)));
            overSampler = std::make_unique<juce::dsp::Oversampling<float>> (1, 
                                                                            overSamplingFactor, 
                                                                            juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR);
            overSampler->initProcessing (static_cast<size_t> (maxSamplesPerBlock));
            
            synthesizer->prepareToPlay (sampleRate * std::pow (2, overSamplingFactor), 
                                        bufferSize * static_cast<int> (std::pow (2, overSamplingFactor)));
            renderBuffer.setSize (1, bufferSize, false, false, true); // Don't re-allocate; maxBufferSize is set in prepareToPlay
            renderBuffer.clear();
            
            storedFactor = overSamplingFactor;
            storedBufferSize = bufferSize;
        }
        
        if (bufferSize != storedBufferSize)
        {
            synthesizer->prepareToPlay (sampleRate * std::pow (2, overSamplingFactor), 
                                        bufferSize * static_cast<int> (std::pow (2, overSamplingFactor)));
            renderBuffer.setSize (1, bufferSize, false, false, true); // Don't re-allocate; maxBufferSize is set in prepareToPlay
            renderBuffer.clear();
            storedBufferSize = bufferSize;
        }
    }

    juce::Array<juce::String> trajectoryStrings {"Ellipse", 
                                                 "Superellipse", 
                                                 "Limacon", 
                                                 "Butterfly", 
                                                 "Scarabaeus", 
                                                 "Squarcle", 
                                                 "Bicorn", 
                                                 "Cornoid", 
                                                 "Epitrochoid 3", "Epitrochoid 5", "Epitrochoid 7", 
                                                 "Hypocycloid 3", "Hypocycloid 5", "Hypocycloid 7", 
                                                 "Gear Curve 3", "Gear Curve 5", "Gear Curve 7"};
    void setCurrentTrajectoryParamFromString (juce::String s) //Need something better here
    {
        for (int i = 0; i < trajectoryStrings.size(); i++)
        {
            if (trajectoryStrings[i] == s)
            {
                parameters.currentTrajectory->setIndex (i);
                return;
            }
        }
        jassertfalse; // Didn't find option
    }
    int getCurrentTrajectoryIndexFromString (juce::String trajectory)
    {
        for (int i = 0; i < trajectoryStrings.size(); i++)
            if (trajectoryStrings[i] == trajectory)
                return i;

    //     jassertfalse;
    //     return 0;
    // }
    juce::StringArray terrainStrings = {"Sinusoidal", 
                                        "System 1", 
                                        "System 2", 
                                        "System 3", 
                                        "System 9", 
                                        "System 11",
                                        "System 12", 
                                        "System 14", 
                                        "System 15"};
    // void setCurrentTerrainFromString (juce::String s)
    // {
    //     for (int i = 0; i < terrainStrings.size(); i++)
    //     {
    //         if (terrainStrings[i] == s)
    //         {
    //             parameters.currentTerrain->setIndex (i);
    //             return;
    //         }
    //     }
    //     jassertfalse; // Didn't find option;
    // }
    // int getCurrentTerrainIndexFromString (juce::String terrain)
    // {
       
    //     for (int i = 0; i < terrainStrings.size(); i++)
    //         if (terrainStrings[i] == terrain)
    //             return i;

    //     jassertfalse;
    //     return 0;
    // }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainProcessor)
};