#pragma once

#include "Panel.h"
#include "ParameterSlider.h"
namespace ti
{
class OutputLevel : public juce::Component 
{
public:
    OutputLevel (juce::ValueTree controlsBranch, 
                 juce::UndoManager& um, 
                 GlobalTimer& gt, 
                 const tp::Parameters& p)
      : state (controlsBranch), 
        undoManager (um), 
        level (p.outputLevel, gt, "Output Level", {-60.0f, 6.0f})
    {
        jassert (state.getType() == id::CONTROLS);
        
        // label.setText ("Output Level", juce::dontSendNotification);
        // label.setJustificationType (juce::Justification::centred);
        // addAndMakeVisible (label);
        level.getSlider().onValueChange = [&]() {state.setProperty (id::outputLevel, level.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (level);
    }
    void paint (juce::Graphics& g) override 
    {
        g.setColour (juce::Colours::black);
        g.drawRect (getLocalBounds());
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        // label.setBounds (b.removeFromTop (20));
        level.setBounds (b.removeFromLeft (b.getWidth()));
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;

    // juce::Label label;
    ParameterSlider level;
};
class Compressor : public juce::Component 
{
public:
    Compressor (juce::ValueTree controlsBranch, 
            juce::UndoManager& um, 
            GlobalTimer& gt, 
            const tp::Parameters& p)
      : state (controlsBranch), 
        undoManager (um), 
        threshold (p.compressorThreshold, gt, "Threshold", {-24.0f, 0.0f}), 
        ratio (p.compressorRatio, gt, "Ratio", {1.0f, 12.0f})
    {
        jassert (state.getType() == id::CONTROLS);
        
        label.setText ("Compressor", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        threshold.getSlider().onValueChange = [&]() {state.setProperty (id::compressionThreshold, threshold.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (threshold);
        ratio.getSlider().onValueChange = [&]() {state.setProperty (id::compressionRatio, ratio.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (ratio);
    }
    void paint (juce::Graphics& g) override 
    {
        g.setColour (juce::Colours::black);
        g.drawRect (getLocalBounds());
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromTop (20));
        auto unitWidth = b.getWidth() / 2.0f;
        threshold.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
        ratio.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;

    juce::Label label;
    ParameterSlider threshold, ratio;
};
class Filter : public juce::Component 
{
public:
    Filter (juce::ValueTree controlsBranch, 
            juce::UndoManager& um, 
            GlobalTimer& gt, 
            const tp::Parameters& p)
      : state (controlsBranch), 
        undoManager (um), 
        frequency (p.filterFrequency, gt, "Frequency", {20.0f, 10000.0f}, 500.0f), 
        resonance (p.filterResonance, gt, "Resonance", {0.0f, 1.0f}), 
        onOff (p.filterOnOff, gt, "")
    {
        jassert (state.getType() == id::CONTROLS);
        
        label.setText ("Filter", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        frequency.getSlider().onValueChange = [&]() {state.setProperty (id::filterFrequency, frequency.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (frequency);
        resonance.getSlider().onValueChange = [&]() {state.setProperty (id::filterResonance, resonance.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (resonance);
        onOff.getToggle().onStateChange = [&]() { state.setProperty (id::filterOnOff, onOff.getToggle().getToggleState(), &undoManager); };
        addAndMakeVisible (onOff);

    }
    void paint (juce::Graphics& g) override 
    {
        g.setColour (juce::Colours::black);
        g.drawRect (getLocalBounds());
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        onOff.setBounds (0, 0, 22, 22);
        label.setBounds (b.removeFromTop (20));
        auto unitWidth = b.getWidth() / 2.0f;
        frequency.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
        resonance.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;

    juce::Label label;
    ParameterSlider frequency, resonance;
    ParameterToggle onOff;
};
class OverSampling : public juce::Component
{
public:
    OverSampling (juce::ValueTree controlsBranch, 
                  juce::UndoManager& um)
      : state (controlsBranch),
        undoManager (um)
    {
        jassert (state.getType() == id::CONTROLS);
        dropDown.addItem ("1X", 1);
        dropDown.addItem ("2X", 2);
        dropDown.addItem ("4X", 3);
        dropDown.addItem ("8X", 4);
        dropDown.addItem ("16X", 5);
        dropDown.setSelectedId (static_cast<int> (state.getProperty (id::oversampling)) + 1, juce::dontSendNotification);
        dropDown.onChange = [&]() 
            {
                auto index = dropDown.getSelectedItemIndex();
                state.setProperty (id::oversampling, index, &undoManager);
            };
        addAndMakeVisible (dropDown);
        label.setText ("Oversampling", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
    }
    void paint (juce::Graphics& g) override 
    {
        auto b = getLocalBounds();
        g.setColour (juce::Colours::black);
        g.drawRect (b);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromTop (20));
        dropDown.setBounds (b.removeFromTop (20));
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;

    juce::Label label;
    juce::ComboBox dropDown;
};
class Envelope : public juce::Component
{
public:
    Envelope (juce::ValueTree trajectoryVariablesBranch, 
              juce::UndoManager& um, 
              GlobalTimer& gt, 
              const tp::Parameters& p)
      : state (trajectoryVariablesBranch),
        undoManager (um),
        parameters (p), 
        envelopeSize (parameters.envelopeSize, gt, "ES"),
        attack (parameters.attack, gt, "Attack", {2.0f, 2000.0f}, 100.0f),
        decay (parameters.decay, gt, "Decay", {2.0f, 1000.0f}, 50.0f), 
        sustain (parameters.sustain, gt, "Sustain", {-24.0f, 0.0f}), 
        release (parameters.release, gt, "Release", {10.0f, 4000.0f}, 800.0f)
    {
        jassert (state.getType() == id::TRAJECTORY_VARIABLES);
        label.setText ("Envelope", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        envelopeSize.getToggle().onStateChange = [&]() { state.setProperty (id::envelopeSize, envelopeSize.getToggle().getToggleState(), &undoManager); };
        addAndMakeVisible (envelopeSize);
        attack.getSlider().onValueChange = [&]() { state.setProperty (id::attack, attack.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (attack);
        decay.getSlider().onValueChange = [&]() { state.setProperty (id::decay, decay.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (decay);
        sustain.getSlider().onValueChange = [&]() { state.setProperty (id::sustain, sustain.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (sustain);
        release.getSlider().onValueChange = [&]() { state.setProperty (id::release, release.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (release);
    }
    void paint (juce::Graphics& g) override 
    {
        auto b = getLocalBounds();
        g.setColour (juce::Colours::black);
        g.drawRect (b);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromTop (20));
        auto unitWidth = b.getWidth() / 43.0f;
        envelopeSize.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 3.0f)));
        attack.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 10.0f)));
        decay.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 10.0f)));
        sustain.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 10.0f)));
        release.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 10.0f)));
    }

private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    const tp::Parameters& parameters;
    juce::Label label;
    ti::ParameterToggle envelopeSize;
    ti::ParameterSlider attack, decay, sustain, release;
};
class ControlPanel : public Panel
{
public:
    ControlPanel (juce::ValueTree synthState, 
                  juce::UndoManager& um, 
                  GlobalTimer& gt, 
                  const tp::Parameters& p)
      : Panel ("Control Panel"), 
        state (synthState), 
        undoManager (um), 
        envelope (state.getChildWithName (id::TRAJECTORY_VARIABLES), undoManager, gt, p), 
        oversampling (state.getChildWithName (id::CONTROLS), undoManager), 
        filter (state.getChildWithName (id::CONTROLS), undoManager, gt, p), 
        compressor (state.getChildWithName (id::CONTROLS), undoManager, gt, p), 
        outputLevel (state.getChildWithName (id::CONTROLS), undoManager, gt, p)
    {
        jassert (state.getType() == id::TERRAINSYNTH);
        addAndMakeVisible (envelope);  
        addAndMakeVisible (oversampling);
        addAndMakeVisible (filter);
        addAndMakeVisible (compressor);
        addAndMakeVisible (outputLevel);
    }
    void resized() override 
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        auto unitWidth = b.getWidth() / 10.0f;
        envelope.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 4.0f)));
        oversampling.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
        filter.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 2.0f)));
        compressor.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 2.0f)));
        outputLevel.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    Envelope envelope;
    OverSampling oversampling;
    Filter filter;
    Compressor compressor;
    OutputLevel outputLevel;
};
}