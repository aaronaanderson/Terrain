#pragma once

#include "Panel.h"
#include "AttachedInterfaces.h"
#include "../DSP/MPEVoiceData.h"
namespace ti
{
class TerrainVariables : public juce::Component 
{
public:
    TerrainVariables (juce::AudioProcessorValueTreeState& vts, 
                      tp::MPEVoiceData& vd)
      : saturation ("Saturation", "TerrainSaturation", vts, vd)
    {
        addAndMakeVisible (saturation);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        saturation.setBounds (b.removeFromTop (b.getHeight()));
    }
private:
    KnobParameterSlider saturation;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TerrainVariables)
};
class TerrainModifierArray : public juce::Component
{
public:
    TerrainModifierArray (juce::AudioProcessorValueTreeState& vts, 
                          tp::MPEVoiceData& vd)
      : aModifier ("a", "TerrainModA", vts, vd),
        bModifier ("b", "TerrainModB", vts, vd),
        cModifier ("c", "TerrainModC", vts, vd),
        dModifier ("d", "TerrainModD", vts, vd)
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
    void setVisibleSliders (int numberVisible)
    {
        jassert (numberVisible <= 4);
        aModifier.setVisible (false);
        bModifier.setVisible (false);
        cModifier.setVisible (false);
        dModifier.setVisible (false);
        switch (numberVisible)
        {
            case 0: break;
            case 1:
                aModifier.setVisible (true);
            break;
            case 2:
                aModifier.setVisible (true);
                bModifier.setVisible (true);
            break;
            case 3:
                aModifier.setVisible (true);
                bModifier.setVisible (true);
                cModifier.setVisible (true);
            break;
            case 4:
                aModifier.setVisible (true);
                bModifier.setVisible (true);
                cModifier.setVisible (true);
                dModifier.setVisible (true);
            break;
        }
    }
private:
    LinearParameterSlider aModifier;
    LinearParameterSlider bModifier;
    LinearParameterSlider cModifier;
    LinearParameterSlider dModifier;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TerrainModifierArray)
};
class TerrainSelector : public juce::Component
{
public:
    TerrainSelector (juce::AudioProcessorValueTreeState& vts, 
                     tp::MPEVoiceData& vd)
      : modifierArray (vts, vd), 
        terrainList ("CurrentTerrain", vts, resetModifierArray)
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
        terrainList.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 2.0f)).withX (2)
                                                                                     .withWidth (b.getWidth() - 4));
        modifierArray.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 8.0f)));
    }
    std::function<void()> resetModifierArray = [&]()
        {
            auto numberOfVisibleSliders = trajectoryNameToVisibleSliders (terrainList.getCurrent());
            modifierArray.setVisibleSliders (numberOfVisibleSliders);
        };
private:
    TerrainModifierArray modifierArray;
    ParameterComboBox terrainList;
    juce::Label terrainListLabel;

    int trajectoryNameToVisibleSliders (juce::String trajectoryName)
    {
        if (trajectoryName == "Sinusoidal") return 2;
        else if (trajectoryName == "System 1") return 2;
        else if (trajectoryName == "System 2") return 2;
        else if (trajectoryName == "System 3") return 1;
        else if (trajectoryName == "System 9") return 2;
        else if (trajectoryName == "System 11") return 3;
        else if (trajectoryName == "System 12") return 2;
        else if (trajectoryName == "System 14") return 3;
        else if (trajectoryName == "System 15") return 1;
        jassertfalse; 
        return 0;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TerrainSelector)       
};
class TerrainPanel : public Panel
{
public:
    TerrainPanel (juce::AudioProcessorValueTreeState& vts, 
                  tp::MPEVoiceData& vd)
      : Panel ("Terrain"), 
        terrainSelector (vts, vd), 
        terrainVariables (vts, vd)
    {
        addAndMakeVisible (terrainSelector);
        addAndMakeVisible (terrainVariables);
        for( int i = 0; i < 2; i++) { lines.add (std::make_unique<Line> (lineThickness));}
        for( auto* line : lines ) { addAndMakeVisible (line); }
    }
    void resized() override
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        auto unitHeight = b.getHeight() / static_cast<float> (12 + 8 + 44 - lines.size() * lineThickness);
        terrainSelector.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 12.0f)));
        lines.getUnchecked( 0 )->setBounds (b.removeFromTop (lineThickness));
        terrainVariables.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 8.0f)));
        lines.getUnchecked( 1 )->setBounds (b.removeFromTop (lineThickness));

    }
private:
    TerrainSelector terrainSelector;
    TerrainVariables terrainVariables;

    struct Line : public juce::Component {
        explicit Line (int w = 2) : thickness (static_cast<float>(w)) {}
        void paint (juce::Graphics& g) {
            auto bounds = getLocalBounds();
            auto* tlaf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel() );
            jassert (tlaf != nullptr);
            g.setColour (tlaf->getBackgroundDark().darker());
            g.drawLine (0.0f, 0.0f, bounds.toFloat().getWidth(), 0.0, thickness);       
        }
        float thickness { 2.0f };
    };
    juce::OwnedArray<Line> lines;
    const int lineThickness { 6 };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TerrainPanel)
};
}