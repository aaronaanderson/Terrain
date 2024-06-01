#pragma once

#include "Panel.h"
#include "ParameterSlider.h"
namespace ti
{
class Envelope : public juce::Component
{
public:
    Envelope (juce::ValueTree trajectoryVariablesBranch, 
              juce::UndoManager& um, 
              GlobalTimer& gt, 
              const tp::Parameters& p)
      : state (trajectoryVariablesBranch),
        parameters (p), 
        undoManager (um),
        envelopeSize (parameters.envelopeSize, gt, "ES"),
        attack (parameters.attack, gt, "Attack", {2.0f, 2000.0f}, 100.0f),
        decay (parameters.decay, gt, "Decay", {2.0f, 1000.0f}, 50.0f), 
        sustain (parameters.sustain, gt, "Sustain", {0.0f, 1.0f}), 
        release (parameters.release, gt, "Release", {10.0f, 4000.0f}, 800.0f)
    {
        jassert (state.getType() == id::TRAJECTORY_VARIABLES);
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
    void resized() override 
    {
        auto b = getLocalBounds();
        envelopeSize.setBounds (b.removeFromLeft (30));
        attack.setBounds (b.removeFromLeft (100));
        decay.setBounds (b.removeFromLeft (100));
        sustain.setBounds (b.removeFromLeft (100));
        release.setBounds (b.removeFromLeft (100));
    }

private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    const tp::Parameters& parameters;
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
        envelope (state.getChildWithName (id::TRAJECTORY_VARIABLES), undoManager, gt, p)
    {
        jassert (state.getType() == id::TERRAINSYNTH);
        addAndMakeVisible (envelope);  
    }
    void resized() override 
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        envelope.setBounds (b.removeFromLeft (430));
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    Envelope envelope;
};
}