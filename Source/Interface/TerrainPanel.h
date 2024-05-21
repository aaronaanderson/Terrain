#pragma once

#include "Panel.h"
#include <juce_data_structures/juce_data_structures.h>
#include "../Identifiers.h"
#include "GlobalTimer.h"
#include "../Parameters.h"
#include "ParameterSlider.h"
namespace ti
{
class TerrainSelector : public juce::Component,
                        private GlobalTimer::Listener, 
                        private juce::AudioProcessorParameter::Listener
{
public:
    TerrainSelector (juce::ValueTree terrainBranch, 
                     juce::UndoManager& um, 
                     GlobalTimer& gt, 
                     const tp::Parameters p)
      : state (terrainBranch),
        undoManager (um),
        parameters (p)
    {
        initializeState();

        addAndMakeVisible (terrainList);
        terrainListLabel.setText ("Current Terrain", juce::NotificationType::dontSendNotification);
        terrainListLabel.setJustificationType (juce::Justification::centred);
        terrainList.onChange = [&]()
        {
            auto selectedName = terrainList.getItemText (terrainList.getSelectedId() - 1);
            state.setProperty (id::currentTerrain, selectedName, &undoManager);
        };
        addAndMakeVisible (terrainListLabel);

        gt.addListener (*this);
        parameters.currentTerrain->addListener (this);
    }
    ~TerrainSelector()
    {
        parameters.currentTerrain->removeListener (this);
    }
    void paint (juce::Graphics& g)
    {
        auto b = getLocalBounds();
        g.drawRect (b);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        terrainListLabel.setBounds (b.removeFromTop (20));
        terrainList.setBounds (b.removeFromTop (20));
    }
    void onTimerCallback() override 
    {
        if (needsRepainted)
        {
            terrainList.setSelectedItemIndex (parameters.currentTerrain->getIndex(), juce::dontSendNotification);
            repaint();
            needsRepainted = false;
        }
    }

private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    const tp::Parameters parameters;

    juce::ComboBox terrainList;
    juce::Label terrainListLabel;
    
    bool needsRepainted = true;

    void initializeState()
    {
        populateMenu();

        auto name = state.getProperty (id::currentTerrain).toString();
        setItemByName (name);
    }
    void populateMenu()
    {
        for (int i = 0; i < state.getNumChildren(); i++)
        {
            auto terrainBranch = state.getChild (i);
            auto name = terrainBranch.getProperty (id::type).toString();
            terrainList.addItem (name, i + 1);
        }
    }
    void setItemByName (juce::String name)
    {
        for (int i = 0; i < terrainList.getNumItems(); i++)
            if (terrainList.getItemText (i) == name)
                terrainList.setSelectedId (i + 1);
    }                
    void parameterValueChanged (int parameterIndex, float newValue) override 
    {
        juce::ignoreUnused (parameterIndex, newValue);
        needsRepainted = true;
    }
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override { juce::ignoreUnused (parameterIndex, gestureIsStarting); }          
};

class TerrainPanel : public Panel
{
public:
    TerrainPanel (juce::ValueTree terrainBranch,
                  juce::UndoManager& um, 
                  GlobalTimer& gt, 
                  const tp::Parameters& p)
      : Panel ("Terrain"), 
        terrainSelector (terrainBranch, um, gt, p)
    {
        addAndMakeVisible (terrainSelector);
    }
    void resized() override
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        terrainSelector.setBounds (b.removeFromTop (40));
    }
private:
    TerrainSelector terrainSelector;
};
}