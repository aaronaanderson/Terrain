#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include "Identifiers.h"
#include "DSP/WaveTerrainSynthesizer.h"
#include "StateHelpers.h"

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
    tp::WaveTerrainSynthesizer& getWaveTerrainSynthesizer() { return *synthesizer.get(); }
private:
    juce::ValueTree state;
    juce::UndoManager undoManager;
    tp::Parameters parameters;
    std::unique_ptr<tp::WaveTerrainSynthesizer> synthesizer;
    std::unique_ptr<juce::dsp::Oversampling<float>> overSampler;
    int storedFactor = 0;

    int samplesPerBlock;
    double sampleRate;

    const juce::String trajectoryNameFromIndex (int i);

    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged,
                                   const juce::Identifier& property) override 
    {
        auto tree = treeWhosePropertyHasChanged;
        if (tree.getType() == id::TRAJECTORIES)
        {
            if (property == id::currentTrajectory)
                setCurrentTrajectoryParamFromString (tree.getProperty (property).toString());
        }else if (tree.getType() == id::TERRAINS)
        {
            if (property == id::currentTerrain)
                setCurrentTerrainFromString (tree.getProperty (property).toString());   
        }else if (tree.getType() == id::MODIFIERS)
        {
            if (tree.getParent().getType() == id::TRAJECTORY)
            {
                if      (property == id::mod_A) parameters.trajectoryModA->setValueNotifyingHost (tree.getProperty (property));
                else if (property == id::mod_B) parameters.trajectoryModB->setValueNotifyingHost (tree.getProperty (property));
                else if (property == id::mod_C) parameters.trajectoryModC->setValueNotifyingHost (tree.getProperty (property));
                else if (property == id::mod_D) parameters.trajectoryModD->setValueNotifyingHost (tree.getProperty (property));
            }else if (tree.getParent().getType() == id::TERRAIN)
            {
                if      (property == id::mod_A) parameters.terrainModA->setValueNotifyingHost (tree.getProperty (property));
                else if (property == id::mod_B) parameters.terrainModB->setValueNotifyingHost (tree.getProperty (property));
                else if (property == id::mod_C) parameters.terrainModC->setValueNotifyingHost (tree.getProperty (property));
                else if (property == id::mod_D) parameters.terrainModD->setValueNotifyingHost (tree.getProperty (property));
            }
        }else if (tree.getType() == id::TRAJECTORY_VARIABLES)
        {
            if      (property == id::size) 
                parameters.trajectorySize->setValueNotifyingHost (tree.getProperty (property));
            else if (property == id::rotation) 
                parameters.trajectoryRotation->setValueNotifyingHost (parameters.trajectoryRotation->convertTo0to1 (tree.getProperty (property)));
            else if (property == id::translation_x)  
                parameters.trajectoryTranslationX->setValueNotifyingHost (parameters.trajectoryTranslationX->convertTo0to1 (tree.getProperty (property)));
            else if (property == id::translation_y) 
                parameters.trajectoryTranslationY->setValueNotifyingHost (parameters.trajectoryTranslationY->convertTo0to1 (tree.getProperty (property)));
            else if (property == id::envelopeSize)
                parameters.envelopeSize->setValueNotifyingHost (static_cast<float> (tree.getProperty (property)));
            else if (property == id::attack) 
                parameters.attack->setValueNotifyingHost (parameters.attack->convertTo0to1 (tree.getProperty (property)));
            else if (property == id::decay) 
                parameters.decay->setValueNotifyingHost (parameters.decay->convertTo0to1 (tree.getProperty (property)));
            else if (property == id::sustain) 
                parameters.sustain->setValueNotifyingHost (parameters.sustain->convertTo0to1 (tree.getProperty (property)));
            else if (property == id::release) 
                parameters.release->setValueNotifyingHost (parameters.release->convertTo0to1 (tree.getProperty (property)));
        }else if (tree.getType() == id::FEEDBACK)
        {
            if      (property == id::feedbackTime)
                parameters.feedbackTime->setValueNotifyingHost (parameters.feedbackTime->convertTo0to1 (tree.getProperty (property)));
            else if (property == id::feedbackScalar)
                parameters.feedbackScalar->setValueNotifyingHost (parameters.feedbackScalar->convertTo0to1 (tree.getProperty (property)));
            else if (property == id::feedbackCompression)
                parameters.feedbackCompression->setValueNotifyingHost (parameters.feedbackCompression->convertTo0to1 (tree.getProperty (property)));
            else if (property == id::feedbackMix)
                parameters.feedbackMix->setValueNotifyingHost (parameters.feedbackMix->convertTo0to1 (tree.getProperty (property)));
        }else if (tree.getType() == id::TERRAIN_VARIABLES)
        {
            if (property == id::terrainSaturation)
                parameters.terrainSaturation->setValueNotifyingHost (parameters.terrainSaturation->convertTo0to1 (tree.getProperty (property)));
        }
        else if (tree.getType() == id::CONTROLS)
        {
            // if (property == id::oversampling)
            //     prepareOversampling();
        }
    }
    void prepareOversampling()
    {
        auto controlsTree = state.getChildWithName (id::CONTROLS);
        auto overSamplingFactor = static_cast<int> (controlsTree.getProperty (id::oversampling));
        if (overSamplingFactor == storedFactor) return;

        storedFactor = overSamplingFactor;
        overSampler = std::make_unique<juce::dsp::Oversampling<float>> (2, 
                                                                       overSamplingFactor, 
                                                                       juce::dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple);
        overSampler->initProcessing (samplesPerBlock);
        synthesizer->prepareToPlay (sampleRate * std::pow (2, overSamplingFactor), 
                                    samplesPerBlock * static_cast<int> (std::pow (2, overSamplingFactor)));
    }

    juce::Array<juce::String> trajectoryStrings {"Ellipse", "Limacon", "Butterfly", "Scarabaeus"};
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

        jassertfalse;
        return 0;
    }
    juce::Array<juce::String> terrainStrings = {"Sinusoidal", "Wiggly", "Wobbly", "System 3", "System 9"};
    void setCurrentTerrainFromString (juce::String s)
    {
        for (int i = 0; i < terrainStrings.size(); i++)
        {
            if (terrainStrings[i] == s)
            {
                parameters.currentTerrain->setIndex (i);
                return;
            }
        }
        jassertfalse; // Didn't find option;
    }
    int getCurrentTerrainIndexFromString (juce::String terrain)
    {
       
        for (int i = 0; i < terrainStrings.size(); i++)
            if (terrainStrings[i] == terrain)
                return i;

        jassertfalse;
        return 0;
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainProcessor)
};