#pragma once

#include "Panel.h"
#include <juce_data_structures/juce_data_structures.h>
#include "../Identifiers.h"
#include "GlobalTimer.h"
namespace ti
{
class TrajectorySelector : public juce::Component, 
                           private juce::ValueTree::Listener, 
                           private GlobalTimer::Listener
{
public:
    TrajectorySelector (juce::ValueTree trajectoryBranch, 
                        juce::UndoManager& um, 
                        GlobalTimer& gt)
      : state (trajectoryBranch),
        undoManager (um)
    {
        state.addListener (this);
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

        state.addListener (this);
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
        if (needsRepainted)
        {
            repaint(); 
            needsRepainted = false;
        }
    }

private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;

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

    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged,
                                   const juce::Identifier& property) override 
    {
            if (property == id::currentTrajectory)
            {
                needsRepainted = true;
                setItemByName (property.toString());
            }
    }
                                
                                   
};
class TrajectoryPanel : public Panel,
                        private GlobalTimer::Listener
{
public:
    TrajectoryPanel (juce::ValueTree trajectoryState, 
                     juce::UndoManager& um, 
                     GlobalTimer gt)
      : Panel ("Trajectory"), 
        state (trajectoryState), 
        undoManager (um),
        trajectorySelector (state, undoManager, gt)
    {
        jassert (state.getType() == id::TRAJECTORIES);
        addAndMakeVisible (trajectorySelector);

        gt.addListener (*this);
    }

    void resized () override 
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        trajectorySelector.setBounds (b.removeFromTop (40));
    }

    void onTimerCallback() override { repaint(); }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    
    TrajectorySelector trajectorySelector;
};
}