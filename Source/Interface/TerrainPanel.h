#pragma once

#include "Panel.h"
#include <juce_data_structures/juce_data_structures.h>
#include "../Identifiers.h"
#include "GlobalTimer.h"
#include "../Parameters.h"
#include "ParameterSlider.h"
namespace ti
{
class TerrainVariables : public juce::Component 
{
public:
    TerrainVariables (juce::ValueTree terrainVariablesBranch, 
                      juce::UndoManager& um, 
                      GlobalTimer& gt, 
                      const tp::Parameters& p)
      : state (terrainVariablesBranch),
        undoManager (um), 
        saturation (p.terrainSaturation, gt, "Saturation", {1.0, 16.0}, 4.0)
    {
        jassert (state.getType() == id::TERRAIN_VARIABLES);
        saturation.getSlider().onValueChange = [&]() {state.setProperty (id::terrainSaturation, saturation.getSlider().getValue(), &undoManager); };
        addAndMakeVisible (saturation);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        saturation.setBounds (b.removeFromTop (40));
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;

    ParameterSlider saturation;
};
class TerrainModifierArray : public juce::Component
{
public:
    TerrainModifierArray (juce::ValueTree terrainState, 
                          juce::UndoManager& um, 
                          GlobalTimer& gt, 
                          const tp::Parameters& p)
      : state (terrainState), 
        undoManager (um), 
        parameters (p), 
        aModifier (parameters.terrainModA, gt, "a", {0.0, 1.0}),
        bModifier (parameters.terrainModB, gt, "b", {0.0, 1.0}),
        cModifier (parameters.terrainModC, gt, "c", {0.0, 1.0}),
        dModifier (parameters.terrainModD, gt, "d", {0.0, 1.0})
    {
        jassert (state.getType() == id::TERRAINS);

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

    const tp::Parameters& parameters;

    ParameterSlider aModifier;
    ParameterSlider bModifier;
    ParameterSlider cModifier;
    ParameterSlider dModifier;

    void setModifier (juce::Identifier mod, float value)
    {
        auto activeTerrainBranch = getCurrentTerrainBranch (state);
        modifierBranch = activeTerrainBranch.getChildWithName (id::MODIFIERS);
        modifierBranch.setProperty (mod, value, &undoManager);
    }
    void initializeState()
    {
        auto activeTerrainBranch = getCurrentTerrainBranch (state);
        modifierBranch = activeTerrainBranch.getChildWithName (id::MODIFIERS);

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
        parameters (p),
        modifierArray (state, undoManager, gt, parameters)
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
        addAndMakeVisible (modifierArray);

        gt.addListener (*this);
        parameters.currentTerrain->addListener (this);
    }
    ~TerrainSelector() override
    {
        parameters.currentTerrain->removeListener (this);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        terrainListLabel.setBounds (b.removeFromTop (20));
        terrainList.setBounds (b.removeFromTop (20));
        modifierArray.setBounds (b.removeFromTop (80));
    }
    void onTimerCallback() override 
    {
        if (needsRepainted)
        {
            terrainList.setSelectedItemIndex (parameters.currentTerrain->getIndex(), juce::dontSendNotification);
            modifierArray.setFromIndex (parameters.currentTerrain->getIndex());
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
    TerrainModifierArray modifierArray;
    
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
    TerrainPanel (juce::ValueTree terrainSynthTree,
                  juce::UndoManager& um, 
                  GlobalTimer& gt, 
                  const tp::Parameters& p)
      : Panel ("Terrain"), 
        state (terrainSynthTree),
        terrainSelector (state.getChildWithName (id::TERRAINS), um, gt, p), 
        terrainVariables (state.getChildWithName (id::TERRAIN_VARIABLES), um, gt, p)
    {
        jassert (state.getType() == id::TERRAINSYNTH);

        addAndMakeVisible (terrainSelector);
        addAndMakeVisible (terrainVariables);
    }
    void resized() override
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        auto unitHeight = b.getHeight() / static_cast<float> (12 + 4 + 44);
        terrainSelector.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 12.0f)));
        terrainVariables.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 4.0f)));
    }
private:
    juce::ValueTree state;
    TerrainSelector terrainSelector;
    TerrainVariables terrainVariables;
};
}