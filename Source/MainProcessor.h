#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include "Identifiers.h"
#include "DSP/WaveTerrainSynthesizer.h"

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

    const tp::Parameters& getParameters() const { return parameters; }
private:
    juce::ValueTree state;
    juce::UndoManager undoManager;
    tp::Parameters parameters;
    std::unique_ptr<tp::WaveTerrainSynthesizer> synthesizer;

    const juce::String trajectoryNameFromIndex (int i);


    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged,
                                   const juce::Identifier& property) override 
    {
        auto tree = treeWhosePropertyHasChanged;
        if (tree.getType() == id::TRAJECTORIES)
        {
            if (property == id::currentTrajectory)
                setCurrentTrajectoryParamFromString (tree.getProperty (property).toString());
        }else if (tree.getType() == id::MODIFIERS)
        {
            if      (property == id::mod_A) parameters.trajectoryModA->setValueNotifyingHost (tree.getProperty (property));
            else if (property == id::mod_B) parameters.trajectoryModB->setValueNotifyingHost (tree.getProperty (property));
            else if (property == id::mod_C) parameters.trajectoryModC->setValueNotifyingHost (tree.getProperty (property));
            else if (property == id::mod_D) parameters.trajectoryModD->setValueNotifyingHost (tree.getProperty (property));
        }else if (tree.getType() == id::TRAJECTORY_VARIABLES)
        {
            if      (property == id::size) parameters.trajectorySize->setValueNotifyingHost (tree.getProperty (property));
            else if (property == id::rotation) parameters.trajectoryRotation->setValueNotifyingHost (tree.getProperty (property));
        }
    }
    void setCurrentTrajectoryParamFromString (juce::String s) //Need something better here
    {
        auto p = parameters.currentTrajectoryParameter;
        if (s == "Ellipse") p->setIndex (0);
        else if (s == "Limacon") p->setIndex (1);
        else if (s == "Butterfly") p->setIndex (2);
        else if (s == "Scarabaeus") p->setIndex (3);
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainProcessor)
};