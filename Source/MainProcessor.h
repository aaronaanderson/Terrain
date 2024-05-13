#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace tp
{
class ChoiceParameter : public juce::AudioParameterChoice
{
public:
    ChoiceParameter(juce::String parameterName, 
                    juce::StringArray iChoices, 
                    juce::String label,
                    std::function<void(int)> newValueFunction = nullptr, 
                    int defaultChoice = 0)
      : choices (iChoices), 
        onNewValue (newValueFunction),
        juce::AudioParameterChoice (parameterName.removeCharacters(" ") + juce::String("Choice"), 
                                    parameterName, 
                                    iChoices, 
                                    defaultChoice,
                                    juce::AudioParameterChoiceAttributes().withLabel (label))
    {
        valueChanged (defaultChoice);
    }

    void valueChanged(int newValue) override
    {
        if (onNewValue == nullptr) return;
        if (newValue == storedValue) return;
        onNewValue (newValue);
        storedValue = newValue;
    }
        
private: 
    std::function<void(int)> onNewValue;
    juce::StringArray choices;
    int storedValue = -1;

};
}
//==============================================================================
class MainProcessor  : public juce::AudioProcessor, 
                       private juce::ValueTree::Listener
{
public:
    //==============================================================================
    MainProcessor();
    ~MainProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
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

    juce::ValueTree& getState() { return state; }
    juce::UndoManager& getUndoManager() { return undoManager; }

private:
    juce::ValueTree state;
    juce::UndoManager undoManager;

    const juce::String trajectoryNameFromIndex (int i);
    tp::ChoiceParameter* currentTrajectoryParameter;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainProcessor)
};