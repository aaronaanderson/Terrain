#pragma once

#include "Panel.h"
#include "AttachedInterfaces.h"
namespace ti
{
class TerrainVariables : public juce::Component 
{
public:
    TerrainVariables (juce::AudioProcessorValueTreeState& vts)
      : saturation ("Saturation", "TerrainSaturation", vts)
    {
        addAndMakeVisible (saturation);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        saturation.setBounds (b.removeFromTop (b.getHeight()));
    }
private:
    ParameterSlider saturation;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TerrainVariables)
};
class TerrainModifierArray : public juce::Component
{
public:
    TerrainModifierArray (juce::AudioProcessorValueTreeState& vts)
      : aModifier ("a", "TerrainModA", vts),
        bModifier ("b", "TerrainModB", vts),
        cModifier ("c", "TerrainModC", vts),
        dModifier ("d", "TerrainModD", vts)
    {
        addAndMakeVisible (aModifier);
        addAndMakeVisible (bModifier);
        addAndMakeVisible (cModifier);
        addAndMakeVisible (dModifier);
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
    ParameterSlider aModifier;
    ParameterSlider bModifier;
    ParameterSlider cModifier;
    ParameterSlider dModifier;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TerrainModifierArray)
};
class TerrainSelector : public juce::Component
{
public:
    TerrainSelector (juce::AudioProcessorValueTreeState& vts)
      : modifierArray (vts), 
        terrainList ("CurrentTererain", vts)
    {
        addAndMakeVisible (terrainList);
        terrainListLabel.setText ("Current Terrain", juce::NotificationType::dontSendNotification);
        terrainListLabel.setJustificationType (juce::Justification::centred);

        addAndMakeVisible (terrainListLabel);
        addAndMakeVisible (modifierArray);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        auto unitHeight = b.getHeight() / static_cast<float> (2 + 2 + 8);
        terrainListLabel.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 2.0f)));
        terrainList.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 2.0f)));
        modifierArray.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 8.0f)));
    }
private:
    TerrainModifierArray modifierArray;
    ParameterComboBox terrainList;
    juce::Label terrainListLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TerrainSelector)       
};
class TerrainPanel : public Panel
{
public:
    TerrainPanel (juce::AudioProcessorValueTreeState& vts)
      : Panel ("Terrain"), 
        terrainSelector (vts), 
        terrainVariables (vts)
    {
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
    TerrainSelector terrainSelector;
    TerrainVariables terrainVariables;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TerrainPanel)
};
}