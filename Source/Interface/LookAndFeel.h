#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

static const juce::Colour accent = juce::Colour::fromString ("#FFff5c00").darker (0.2f);
static const juce::Colour base = juce::Colour::fromString ("#FF00a3ff").darker (1.0f);
static const juce::Colour background = base.darker (0.5f);

class TerrainLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TerrainLookAndFeel()
      : juce::LookAndFeel_V4(juce::LookAndFeel_V4::ColourScheme(
            background, // windowBackground = 0,
            base, // widgetBackground,
            base, // menuBackground,
            juce::Colours::black, // outline,
            juce::Colours::white, // defaultText,
            background, // defaultFill,
            juce::Colours::white, // highlightedText,
            base, // highlightedFill,
            juce::Colours::white // menuText,
        ))
    {
        setColour (juce::Slider::thumbColourId, accent);
    }
    
};