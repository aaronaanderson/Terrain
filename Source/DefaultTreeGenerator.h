#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include "Identifiers.h"

struct SettingsTree
{
    static juce::ValueTree create()
    {
        juce::ValueTree tree (id::PRESET_SETTINGS);
        tree.setProperty (id::presetRandomizationScale, 0.2f, nullptr);
        tree.setProperty (id::oversampling, 1, nullptr);
        tree.setProperty (id::pitchBendRange, 2.0f, nullptr);
        // tree.setProperty (id::presetName, "Default", nullptr);
        return tree;
    }
};
