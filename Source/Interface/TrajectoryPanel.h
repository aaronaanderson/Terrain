#pragma once

#include "Panel.h"
#include <juce_data_structures/juce_data_structures.h>
#include "../Identifiers.h"
#include "GlobalTimer.h"
#include "../Parameters.h"
#include "ParameterSlider.h"

#include "StateHelpers.h"
namespace ti
{
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
        parameters (p)
    {

        initializeState();

        addAndMakeVisible (trajectoryList);
        trajectoryListLabel.setText ("Current Trajectory", juce::NotificationType::dontSendNotification);
        trajectoryListLabel.setJustificationType (juce::Justification::centred);
        trajectoryList.onChange = [&]()
        {
            auto selectedName = trajectoryList.getItemText (trajectoryList.getSelectedId() - 1);
            state.setProperty (id::currentTrajectory, selectedName, &undoManager);
        };
        addAndMakeVisible (trajectoryListLabel);

        gt.addListener (*this);
        parameters.currentTrajectory->addListener (this);
    }
    ~TrajectorySelector()
    {
        parameters.currentTrajectory->removeListener (this);
    }
    void paint (juce::Graphics& g)
    {
        auto b = getLocalBounds();
        g.drawRect (b);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        trajectoryListLabel.setBounds (b.removeFromTop (20));
        trajectoryList.setBounds (b.removeFromTop (20));
    }
    void onTimerCallback() override 
    {
        if (needsRepainted)
        {
            trajectoryList.setSelectedItemIndex (parameters.currentTrajectory->getIndex(), juce::dontSendNotification);
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
        aModifier (parameters.trajectoryModA, um, gt, "a", {0.0, 1.0}),
        bModifier (parameters.trajectoryModB, um, gt, "b", {0.0, 1.0}),
        cModifier (parameters.trajectoryModC, um, gt, "c", {0.0, 1.0}),
        dModifier (parameters.trajectoryModD, um, gt, "d", {0.0, 1.0})
    {
        jassert (state.getType() == id::TRAJECTORIES);
        
        state.addListener (this);

        aModifier.getSlider().onValueChange = [&](){modifierBranch.setProperty (id::mod_A, aModifier.getSlider().getValue(), &undoManager);};
        addAndMakeVisible (aModifier);
        bModifier.getSlider().onValueChange = [&](){modifierBranch.setProperty (id::mod_B, aModifier.getSlider().getValue(), &undoManager);};
        addAndMakeVisible (bModifier);
        cModifier.getSlider().onValueChange = [&](){modifierBranch.setProperty (id::mod_C, aModifier.getSlider().getValue(), &undoManager);};
        addAndMakeVisible (cModifier);
        dModifier.getSlider().onValueChange = [&](){modifierBranch.setProperty (id::mod_D, aModifier.getSlider().getValue(), &undoManager);};
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
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged,
                                   const juce::Identifier& property) override 
    {
        auto tree = treeWhosePropertyHasChanged;
        if (tree.getType() == id::TRAJECTORIES)
            if (property == id::currentTrajectory)
                initializeState();
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
        juce::ValueTree activeTrajectory = getCurrentTrajectoryBranch (state);
        
        auto modifiersBranch = activeTrajectory.getChildWithName (id::MODIFIERS);
        if (modifiersBranch.getProperty (mod) != juce::var()) // if the property is present
            return true;
        
        return false;
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
        globalTimer (gt),
        parameters (p), 
        size (parameters.trajectorySize, um, gt, "Size", {0.0, 1.0}),
        rotation (parameters.trajectoryRotation, um, gt, "Rotation", {0.0, juce::MathConstants<double>::twoPi}),
        translation_x (parameters.trajectoryTranslationX, um, gt, "Translation X", {-1.0f, 1.0f}),
        translation_y (parameters.trajectoryTranslationY, um, gt, "Translation Y", {-1.0f, 1.0f})
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
        size.setBounds (b.removeFromTop (40));
        rotation.setBounds (b.removeFromTop (40));
        translation_x.setBounds (b.removeFromTop (40));
        translation_y.setBounds (b.removeFromTop (40));
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    GlobalTimer& globalTimer;
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
        trajectorySelector (state.getChildWithName (id::TRAJECTORIES), undoManager, gt, p),
        modifierArray (state.getChildWithName (id::TRAJECTORIES), undoManager, gt, p), 
        trajectoryVariables (state.getChildWithName (id::TRAJECTORY_VARIABLES), undoManager, gt, p)
    {
        jassert (state.getType() == id::TERRAINSYNTH);
        addAndMakeVisible (trajectorySelector);
        addAndMakeVisible (modifierArray);
        addAndMakeVisible (trajectoryVariables);
    }

    void resized () override 
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        trajectorySelector.setBounds (b.removeFromTop (40));
        modifierArray.setBounds (b.removeFromTop (80));
        trajectoryVariables.setBounds (b);
    }

private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    
    TrajectorySelector trajectorySelector;
    ModifierArray modifierArray;
    TrajectoryVariables trajectoryVariables;
};
}