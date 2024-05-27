#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

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
        g.setColour (juce::Colours::black);
        auto b = getLocalBounds();
        g.drawRect (b);

        g.drawRect (b.removeFromTop (20));
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        nameLabel.setBounds (b.removeFromTop (labelHeight));
    }
    juce::Rectangle<int> getAdjustedBounds() const noexcept  
    {
        auto b = getLocalBounds();
        auto leftOverBounds = b.removeFromTop (labelHeight);
        return leftOverBounds;
    }
private:
    juce::String name;
    int labelHeight = 20;
    juce::Label nameLabel;
};
}