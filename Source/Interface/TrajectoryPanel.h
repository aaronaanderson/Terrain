#pragma once

#include "Panel.h"
#include <juce_data_structures/juce_data_structures.h>
#include "../Identifiers.h"
#include "GlobalTimer.h"
#include "../Parameters.h"
namespace ti
{
class TrajectorySelector : public juce::Component,
                           private GlobalTimer::Listener
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
        trajectoryList.setSelectedItemIndex (parameters.currentTrajectoryParameter->getIndex(), juce::dontSendNotification);
        repaint();
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
};
struct LabelSlider : public juce::Component
{
    LabelSlider (juce::String labelText, juce::Range<double> range)
    {
        label.setText (labelText, juce::dontSendNotification);
        slider.setRange (range, 0.0);
        slider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 20, 20);
        addAndMakeVisible (label);
        addAndMakeVisible (slider);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromLeft (20));
        slider.setBounds (b);
    }
    void setValue (double v) { slider.setValue (v, juce::NotificationType::dontSendNotification); }
    juce::Slider& getSlider() { return slider; }
private:
    juce::Label label;
    juce::Slider slider;
};
static juce::ValueTree getCurrentTrajectoryBranch (juce::ValueTree trajectoriesBranch)
{
    jassert (trajectoriesBranch.getType() == id::TRAJECTORIES);
    auto trajectoryType = trajectoriesBranch.getProperty (id::currentTrajectory).toString();
    for (int i = 0; i < trajectoriesBranch.getNumChildren(); i++)
        if (trajectoriesBranch.getChild (i).getProperty (id::type).toString() == trajectoryType)
            return trajectoriesBranch.getChild (i);

    jassertfalse;
    return {};
}

class ModifierArray : public juce::Component,
                      private GlobalTimer::Listener, 
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
        aModifier ("a", {0.0, 1.0}),
        bModifier ("b", {0.0, 1.0}),
        cModifier ("c", {0.0, 1.0}),
        dModifier ("d", {0.0, 1.0})
    {
        jassert (state.getType() == id::TRAJECTORIES);
        
        state.addListener (this);
        gt.addListener (*this);

        aModifier.getSlider().onValueChange = [&]() {setModifier (id::mod_A, static_cast<float>(aModifier.getSlider().getValue())); };
        addAndMakeVisible (aModifier);
        bModifier.getSlider().onValueChange = [&]() {setModifier (id::mod_B, static_cast<float>(bModifier.getSlider().getValue())); };
        addAndMakeVisible (bModifier);
        cModifier.getSlider().onValueChange = [&]() {setModifier (id::mod_C, static_cast<float>(cModifier.getSlider().getValue())); };
        addAndMakeVisible (cModifier);
        dModifier.getSlider().onValueChange = [&]() {setModifier (id::mod_D, static_cast<float>(dModifier.getSlider().getValue())); };
        addAndMakeVisible (dModifier);

        initializeState();
    }

    void onTimerCallback() override { repaint(); }
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
    juce::UndoManager& undoManager;

    tp::Parameters parameters;

    LabelSlider aModifier;
    LabelSlider bModifier;
    LabelSlider cModifier;
    LabelSlider dModifier;
    
    void setModifier (juce::Identifier mod, float value)
    {
        auto activeTrajectoryBranch = getCurrentTrajectoryBranch (state);
        auto modifierBranch = activeTrajectoryBranch.getChildWithName (id::MODIFIERS);
        modifierBranch.setProperty (mod, value, &undoManager);
    }
    void initializeState()
    {
        auto activeTrajectoryBranch = getCurrentTrajectoryBranch (state);
        auto modifierBranch = activeTrajectoryBranch.getChildWithName (id::MODIFIERS);
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
class TrajectoryPanel : public Panel,
                        private GlobalTimer::Listener
{
public:
    TrajectoryPanel (juce::ValueTree trajectoryState, 
                     juce::UndoManager& um, 
                     GlobalTimer& gt, 
                     const tp::Parameters& p)
      : Panel ("Trajectory"), 
        state (trajectoryState), 
        undoManager (um),
        trajectorySelector (state, undoManager, gt, p),
        modifierArray      (state, undoManager, gt, p)
    {
        jassert (state.getType() == id::TRAJECTORIES);
        addAndMakeVisible (trajectorySelector);
        addAndMakeVisible (modifierArray);
        gt.addListener (*this);
    }

    void resized () override 
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        trajectorySelector.setBounds (b.removeFromTop (40));
        modifierArray.setBounds (b.removeFromTop (80));
    }

    void onTimerCallback() override { repaint(); }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    
    TrajectorySelector trajectorySelector;
    ModifierArray modifierArray;
};
}