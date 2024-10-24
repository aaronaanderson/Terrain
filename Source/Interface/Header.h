#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "../Identifiers.h"

namespace ti{

class PresetComponent : public juce::Component
{
public:
    PresetComponent()
    {
        addAndMakeVisible (presets);
        presetLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (presetLabel);
        addAndMakeVisible (saveButton);

        randomizeAmountSlider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 20, 20);
        addAndMakeVisible (randomizeAmountSlider);
        addAndMakeVisible (randomizeButton);
    }  
    void paint (juce::Graphics& g) override 
    {
        g.setColour (juce::Colours::black);
        auto b = getLocalBounds();
        g.drawRect (b, 2);
    }
    void resized() override
    {
        auto b = getLocalBounds().reduced (2);
        auto twoThirds = b.getWidth() * 2 / 3;

        auto p = b.removeFromLeft (twoThirds);
        auto quarterP = p.getWidth() / 4;
        auto pMenu = p.removeFromLeft (quarterP * 3);
        presetLabel.setBounds (pMenu.removeFromTop (p.getHeight() / 2));
        presets.setBounds (pMenu);
        saveButton.setBounds (p.reduced (8));

        randomizeButton.setBounds (b.removeFromTop (p.getHeight() / 2));
        randomizeAmountSlider.setBounds (b);
    }
private:
    juce::ComboBox presets;
    juce::Label presetLabel {"Preset", "Preset"};
    juce::TextButton saveButton {"Save"};

    juce::Slider randomizeAmountSlider;
    juce::TextButton randomizeButton {"Randomize"};
};
class Header : public juce::Component
{
public:
    Header()
    {
        addAndMakeVisible (presetComponent);
    }
    void paint (juce::Graphics& g) override 
    {
        g.setColour (juce::Colours::black);
        auto b = getLocalBounds();
        g.drawRect (b, 2);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        auto oneThird = b.getWidth() / 3;

        auto presetRect = juce::Rectangle<int>().withHeight(b.getHeight())
                                                .withWidth (oneThird);
        presetComponent.setBounds (presetRect.withCentre (b.getCentre()));
                                                          
    }
private:
    PresetComponent presetComponent;
};
} // end namespace ti