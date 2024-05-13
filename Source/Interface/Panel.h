#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace ti
{
class Panel : public juce::Component
{
public:
    Panel (juce::String panelName)
      : name (panelName)
    {}
    void paint (juce::Graphics& g) override 
    {
        auto b = getLocalBounds();
        g.drawRect (b);
        auto titleBounds = b.removeFromTop (20);
        g.drawRect (titleBounds);
        g.drawText (name, 
                    titleBounds, 
                    juce::Justification::centred);
    }
private:
    juce::String name;
};
}