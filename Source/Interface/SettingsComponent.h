#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>

namespace ti
{
class SettingsComponent : public juce::Component
{
public:
    SettingsComponent (juce::ValueTree settingsBranch)
      :  settings (settingsBranch)
    {
        jassert (settings.getType() == id::PRESET_SETTINGS);
    }
private:
    juce::ValueTree settings;
};
}