#pragma once

#include "Panel.h"
#include <juce_data_structures/juce_data_structures.h>
#include "../Identifiers.h"
#include "GlobalTimer.h"
#include "../Parameters.h"
#include "ParameterSlider.h"

#include "../StateHelpers.h"
namespace ti
{
class ModifierArray : public juce::Component,
                      private juce::ValueTree::Listener
{
public:
    ModifierArray (juce::ValueTree trajectoryState, 
                   juce::UndoManager& um, 
                   GlobalTimer& gt, 
                   const tp::Parameters& p)
      : state (trajectoryState), 
        undoManager (um), 
        parameters (p), 
        aModifier (parameters.trajectoryModA, gt, "a", {0.0, 1.0}),
        bModifier (parameters.trajectoryModB, gt, "b", {0.0, 1.0}),
        cModifier (parameters.trajectoryModC, gt, "c", {0.0, 1.0}),
        dModifier (parameters.trajectoryModD, gt, "d", {0.0, 1.0})
    {
        jassert (state.getType() == id::TRAJECTORIES);
        state.addListener (this);

        aModifier.getSlider().onValueChange = [&](){modifierBranch.setProperty (id::mod_A, aModifier.getSlider().getValue(), &undoManager);};
        addAndMakeVisible (aModifier);
        bModifier.getSlider().onValueChange = [&](){modifierBranch.setProperty (id::mod_B, bModifier.getSlider().getValue(), &undoManager);};
        addAndMakeVisible (bModifier);
        cModifier.getSlider().onValueChange = [&](){modifierBranch.setProperty (id::mod_C, cModifier.getSlider().getValue(), &undoManager);};
        addAndMakeVisible (cModifier);
        dModifier.getSlider().onValueChange = [&](){modifierBranch.setProperty (id::mod_D, dModifier.getSlider().getValue(), &undoManager);};
        addAndMakeVisible (dModifier);

        initializeState();
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        auto quarterHeight = b.getHeight() / 4;
        aModifier.setBounds (b.removeFromTop (quarterHeight));
        bModifier.setBounds (b.removeFromTop (quarterHeight));
        cModifier.setBounds (b.removeFromTop (quarterHeight));
        dModifier.setBounds (b.removeFromTop (quarterHeight));
    }
    void setFromIndex (int index)
    {
        auto activeTrajectoryBranch = state.getChild (index);
        modifierBranch = activeTrajectoryBranch.getChildWithName (id::MODIFIERS);
        resetLayout();
    }
private:
    juce::ValueTree state;
    juce::ValueTree modifierBranch;
    juce::UndoManager& undoManager;

    tp::Parameters parameters;

    ParameterSlider aModifier;
    ParameterSlider bModifier;
    ParameterSlider cModifier;
    ParameterSlider dModifier;

    void setModifier (juce::Identifier mod, float value)
    {
        auto activeTrajectoryBranch = getCurrentTrajectoryBranch (state);
        modifierBranch = activeTrajectoryBranch.getChildWithName (id::MODIFIERS);
        modifierBranch.setProperty (mod, value, &undoManager);
    }
    void initializeState()
    {
        auto activeTrajectoryBranch = getCurrentTrajectoryBranch (state);
        modifierBranch = activeTrajectoryBranch.getChildWithName (id::MODIFIERS);
        aModifier.setValue (modifierBranch.getProperty (id::mod_A));
        bModifier.setValue (modifierBranch.getProperty (id::mod_B));
        cModifier.setValue (modifierBranch.getProperty (id::mod_C));
        dModifier.setValue (modifierBranch.getProperty (id::mod_D));

        resetLayout();
    }
    void resetLayout()
    {
        scanForMod (id::mod_A) ? aModifier.setVisible (true) : aModifier.setVisible (false);
        scanForMod (id::mod_B) ? bModifier.setVisible (true) : bModifier.setVisible (false);
        scanForMod (id::mod_C) ? cModifier.setVisible (true) : cModifier.setVisible (false);
        scanForMod (id::mod_D) ? dModifier.setVisible (true) : dModifier.setVisible (false);
    }
    bool scanForMod (juce::Identifier mod)
    {
        if (modifierBranch.getProperty (mod) != juce::var()) // if the property is present
            return true;
        
        return false;
    }      
};

class TrajectorySelector : public juce::Component,
                           private GlobalTimer::Listener, 
                           private juce::AudioProcessorParameter::Listener
{
public:
    TrajectorySelector (juce::ValueTree trajectoryBranch, 
                        juce::UndoManager& um, 
                        GlobalTimer& gt, 
                        const tp::Parameters p)
      : state (trajectoryBranch),
        undoManager (um),
        parameters (p), 
        modifierArray (state, undoManager, gt, parameters)
    {
        jassert (state.getType() == id::TRAJECTORIES);
        trajectoryListLabel.setText ("Current Trajectory", juce::NotificationType::dontSendNotification);
        trajectoryListLabel.setJustificationType (juce::Justification::centred);
        trajectoryList.onChange = [&]()
        {
            auto selectedName = trajectoryList.getItemText (trajectoryList.getSelectedId() - 1);
            state.setProperty (id::currentTrajectory, selectedName, &undoManager);
        };
        addAndMakeVisible (trajectoryList);
        addAndMakeVisible (trajectoryListLabel);
        addAndMakeVisible (modifierArray);

        gt.addListener (*this);
        parameters.currentTrajectory->addListener (this);
        initializeState();
    }
    ~TrajectorySelector() override
    {
        parameters.currentTrajectory->removeListener (this);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        auto unitHeight = b.getHeight() / static_cast<float> (2 + 2 + 8);
        trajectoryListLabel.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 2.0f)));
        trajectoryList.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 2.0f)));
        modifierArray.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 8.0f)));
    }
    void onTimerCallback() override 
    {
        if (needsRepainted)
        {
            trajectoryList.setSelectedItemIndex (parameters.currentTrajectory->getIndex(), juce::dontSendNotification);
            modifierArray.setFromIndex(parameters.currentTrajectory->getIndex());
            repaint();
            needsRepainted = false;
        }
    }

private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    const tp::Parameters parameters;

    juce::ComboBox trajectoryList;
    juce::Label trajectoryListLabel;
    ModifierArray modifierArray;
    
    bool needsRepainted = true;

    void initializeState()
    {
        populateMenu();

        auto name = state.getProperty (id::currentTrajectory).toString();
        setItemByName (name);
    }
    void populateMenu()
    {
        for (int i = 0; i < state.getNumChildren(); i++)
        {
            auto trajectoryBranch = state.getChild (i);
            auto name = trajectoryBranch.getProperty (id::type).toString();
            trajectoryList.addItem (name, i + 1);
        }
    }
    void setItemByName (juce::String name)
    {
        for (int i = 0; i < trajectoryList.getNumItems(); i++)
            if (trajectoryList.getItemText (i) == name)
                trajectoryList.setSelectedId (i + 1);
    }                
    void parameterValueChanged (int parameterIndex, float newValue) override 
    {
        juce::ignoreUnused (parameterIndex, newValue);
        needsRepainted = true;
    }
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override { juce::ignoreUnused (parameterIndex, gestureIsStarting); }          
};
class FeedbackPanel : public juce::Component
{
public:
    FeedbackPanel (juce::ValueTree feedbackBranch, 
                   juce::UndoManager& um, 
                   GlobalTimer& gt, 
                   const tp::Parameters& p)
      : state (feedbackBranch), 
        undoManager (um), 
        parameters (p),
        time (parameters.feedbackTime, gt, "Time", {0.0, 2000.0}, 250.0), 
        feedback (parameters.feedbackScalar, gt, "Feedback", {0.0, 0.9999}, 0.8), 
        mix (parameters.feedbackMix, gt, "Mix", {0.0, 1.0}),
        compression (parameters.feedbackCompression, gt, "Compression", {1.0, 20.0})
    {
        jassert (state.getType() == id::FEEDBACK);

        label.setText ("Trajectory Feedback", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        
        time.getSlider().onValueChange = [&]() { state.setProperty (id::feedbackTime, time.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (time);
        feedback.getSlider().onValueChange = [&]() { state.setProperty (id::feedbackScalar, feedback.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (feedback);
        compression.getSlider().onValueChange = [&]() { state.setProperty (id::feedbackCompression, compression.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (compression);
        mix.getSlider().onValueChange = [&]() { state.setProperty (id::feedbackMix, mix.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (mix);

        initializeState();
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        auto unitHeight = b.getHeight() / static_cast<float> (2 + 4 + 4 + 4 + 4);
        label.setBounds (b.removeFromTop(static_cast<int> (unitHeight * 2.0f)));
        time.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 4.0f)));
        feedback.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 4.0f)));
        compression.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 4.0f)));
        mix.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 4.0f)));
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    const tp::Parameters& parameters;

    juce::Label label;
    ParameterSlider time, feedback, mix, compression;

    void initializeState()
    {
        time.setValue (state.getProperty (id::feedbackTime));
        feedback.setValue (state.getProperty (id::feedbackScalar));
        mix.setValue (state.getProperty (id::feedbackMix));
        compression.setValue (state.getProperty (id::feedbackCompression));
    }
};
class TrajectoryVariables : public juce::Component 
{
public:
    TrajectoryVariables (juce::ValueTree trajectoryVariablesBranch, 
                         juce::UndoManager& um, 
                         GlobalTimer& gt, 
                         const tp::Parameters& p)
      : state (trajectoryVariablesBranch), 
        undoManager (um),
        parameters (p), 
        size (parameters.trajectorySize, gt, "Size", {0.0, 1.0}),
        rotation (parameters.trajectoryRotation, gt, "Rotation", {0.0, juce::MathConstants<double>::twoPi}),
        translation_x (parameters.trajectoryTranslationX, gt, "Translation X", {-1.0f, 1.0f}),
        translation_y (parameters.trajectoryTranslationY, gt, "Translation Y", {-1.0f, 1.0f})
    {
        jassert (state.getType() == id::TRAJECTORY_VARIABLES);

        size.getSlider().onValueChange = [&](){ state.setProperty (id::size, size.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (size);
        rotation.getSlider().onValueChange = [&](){ state.setProperty (id::rotation, rotation.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (rotation);
        translation_x.getSlider().onValueChange = [&](){ state.setProperty (id::translation_x, translation_x.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (translation_x);
        translation_y.getSlider().onValueChange = [&](){ state.setProperty (id::translation_y, translation_y.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (translation_y);

        initializeState();
    }

    void resized() override 
    {
        auto b = getLocalBounds();
        auto unitHeight = b.getHeight() / static_cast<float> (4);
        size.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        rotation.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        translation_x.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        translation_y.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    const tp::Parameters& parameters;

    ParameterSlider size;
    ParameterSlider rotation;
    ParameterSlider translation_x;
    ParameterSlider translation_y;

    void initializeState()
    {
        size.setValue (state.getProperty (id::size));
        rotation.setValue (state.getProperty (id::rotation));
        translation_x.setValue (state.getProperty (id::translation_x));
        translation_y.setValue (state.getProperty (id::translation_y));
    }
};
class MeanderancePanel : public juce::Component
{
public:
    MeanderancePanel (juce::ValueTree trajectoryVariableBranch, 
                      juce::UndoManager& um, 
                      GlobalTimer& gt, 
                      const tp::Parameters& p)
      : state (trajectoryVariableBranch),
        undoManager (um), 
        scale (p.meanderanceScale, gt, "Scale", {0.0f, 1.0f}),
        speed (p.meanderanceSpeed, gt, "Speed", {0.0f, 1.0f})
    {
        jassert (state.getType() == id::TRAJECTORY_VARIABLES);
        
        label.setText ("Meanderance", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        scale.getSlider().onValueChange = [&](){ state.setProperty (id::meanderanceScale, scale.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (scale);
        speed.getSlider().onValueChange = [&](){ state.setProperty (id::meanderanceSpeed, speed.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (speed);
    }
    void resized()
    {
        auto b = getLocalBounds();
        auto unitHeight = b.getHeight() / static_cast<float> (2 + 4 + 4);
        label.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 2.0f)));
        scale.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 4.0f)));
        speed.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 4.0f)));
    }

private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    
    juce::Label label;
    ParameterSlider scale, speed;
};
class TrajectoryPanel : public Panel
{
public:
    TrajectoryPanel (juce::ValueTree trajectoryState, 
                     juce::UndoManager& um, 
                     GlobalTimer& gt, 
                     const tp::Parameters& p)
      : Panel ("Trajectory"), 
        state (trajectoryState), 
        undoManager (um),
        // modifierArray (state.getChildWithName (id::TRAJECTORIES), undoManager, gt, p), 
        trajectorySelector (state.getChildWithName (id::TRAJECTORIES), undoManager, gt, p),
        trajectoryVariables (state.getChildWithName (id::TRAJECTORY_VARIABLES), undoManager, gt, p),
        meanderancePanel (state.getChildWithName (id::TRAJECTORY_VARIABLES), undoManager, gt, p),
        feedbackPanel (state.getChildWithName (id::TRAJECTORY_VARIABLES).getChildWithName (id::FEEDBACK), undoManager, gt, p)
    {
        jassert (state.getType() == id::TERRAINSYNTH);
        addAndMakeVisible (trajectorySelector);
        addAndMakeVisible (trajectoryVariables);
        addAndMakeVisible (meanderancePanel);
        addAndMakeVisible (feedbackPanel);
    }

    void resized () override 
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        auto unitHeight = b.getHeight() / static_cast<float> ((12 + 16 + 10 + 22));
        trajectorySelector.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 12.0f)));
        trajectoryVariables.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 16.0f)));
        meanderancePanel.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 10.0f)));
        feedbackPanel.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 22.0f)));
    }

private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    
    TrajectorySelector trajectorySelector;
    TrajectoryVariables trajectoryVariables;
    MeanderancePanel meanderancePanel;
    FeedbackPanel feedbackPanel;
};
}