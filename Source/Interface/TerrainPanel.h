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

class TerrainModifierArray : public juce::Component,
                             private juce::ValueTree::Listener
{
public:
    TerrainModifierArray (juce::ValueTree terrainState, 
                          juce::UndoManager& um, 
                          GlobalTimer& gt, 
                          const tp::Parameters& p)
      : state (terrainState), 
        undoManager (um), 
        parameters (p), 
        aModifier (parameters.terrainModA, um, gt, "a", {0.0, 1.0}),
        bModifier (parameters.terrainModB, um, gt, "b", {0.0, 1.0}),
        cModifier (parameters.terrainModC, um, gt, "c", {0.0, 1.0}),
        dModifier (parameters.terrainModD, um, gt, "d", {0.0, 1.0})
    {
        jassert (state.getType() == id::TERRAINS);
        
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
        auto activeTerrainBranch = getCurrentTerrainBranch (state);
        modifierBranch = activeTerrainBranch.getChildWithName (id::MODIFIERS);
        modifierBranch.setProperty (mod, value, &undoManager);
    }
    void initializeState()
    {
        auto activeTerrainBranch = getCurrentTerrainBranch (state);
        modifierBranch = activeTerrainBranch.getChildWithName (id::MODIFIERS);
        std::cout << activeTerrainBranch.toXmlString() << std::endl;
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
        if (tree.getType() == id::TERRAINS)
            if (property == id::currentTerrain)
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
        juce::ValueTree activeTerrain = getCurrentTerrainBranch (state);
        
        auto modifiersBranch = activeTerrain.getChildWithName (id::MODIFIERS);
        if (modifiersBranch.getProperty (mod) != juce::var()) // if the property is present
            return true;
        
        return false;
    }
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
        undoManager (um),
        terrainSelector (state.getChildWithName (id::TERRAINS), um, gt, p), 
        modifierArray (state.getChildWithName (id::TERRAINS), um, gt, p)
    {
        jassert (state.getType() == id::TERRAINSYNTH);

        addAndMakeVisible (terrainSelector);
        addAndMakeVisible (modifierArray);
    }
    void resized() override
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        terrainSelector.setBounds (b.removeFromTop (40));
        modifierArray.setBounds (b.removeFromTop (80));
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;

    TerrainSelector terrainSelector;
    TerrainModifierArray modifierArray;
};
}