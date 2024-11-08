#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel.h"
namespace ti
{
class Panel : public juce::Component
{
public:
    Panel (juce::String panelName)
      : name (panelName)
    {
        nameLabel.setText (name, juce::NotificationType::dontSendNotification);
        nameLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (nameLabel);
    }
    void paint (juce::Graphics& g) override 
    {
        auto b = getLocalBounds();
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        g.setColour (laf->getBackgroundDark());
        g.drawRect (b, 2);

        g.drawRect (b.removeFromTop (20));
        auto theFont = g.getCurrentFont();
        theFont.setHeight(static_cast<float> (proportionOfWidth(0.03125f)));
        g.setFont(theFont);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        nameLabel.setBounds (b.removeFromTop (labelHeight));
    }
    juce::Rectangle<int> getAdjustedBounds() const noexcept  
    {
        auto b = getLocalBounds();
        b.removeFromTop (labelHeight);
        return b;
    }
private:
    juce::String name;
    int labelHeight = 20;
    juce::Label nameLabel;
};
}