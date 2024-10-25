#pragma once 

#include <juce_gui_basics/juce_gui_basics.h>

namespace ti 
{
class PopUpWindow : public juce::DocumentWindow
{
public:
    PopUpWindow (juce::Component* contentComponent,
                 std::function<void()> closeWindow,
                 juce::String name, 
                 juce::Colour colour, 
                 int requiredButtons)
      : DocumentWindow (name, colour, requiredButtons),
        onClose (closeWindow)
    {
        setResizeLimits (4, 4, 32768, 32768);
        setContentOwned (contentComponent, false);
    }
private:
    void closeButtonPressed() override { onClose(); }
    std::function<void()> onClose = nullptr;
};
}