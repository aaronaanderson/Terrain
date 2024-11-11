#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include "Identifiers.h"

struct SettingsTree
{
    struct DefaultSettings
    {
        static constexpr float presetRandomizationScale = 0.2f;
        static constexpr int oversampling = 1;
        static constexpr float pitchBendRange = 2.0f;
        static constexpr bool noteOnOrContinuous = false;
    };
    static juce::ValueTree create()
    {
        juce::ValueTree tree (id::PRESET_SETTINGS);
        tree.setProperty (id::presetRandomizationScale, DefaultSettings::presetRandomizationScale, nullptr);
        tree.setProperty (id::oversampling, DefaultSettings::oversampling, nullptr);
        tree.setProperty (id::pitchBendRange, DefaultSettings::pitchBendRange, nullptr);
        
        // true = continuous
        tree.setProperty (id::noteOnOrContinuous, DefaultSettings::noteOnOrContinuous, nullptr);
        return tree;
    }
};

struct EphemeralStateTree
{
    struct DefaultSettings
    {
        static constexpr bool tuningSystemConnected = false;
        // static constexpr char* tuningSystemName = "12-TET";
    };
    static juce::ValueTree create()
    {
        juce::ValueTree tree (id::EPHEMERAL_STATE);
        tree.setProperty (id::tuningSystemConnected, DefaultSettings::tuningSystemConnected, nullptr);
        tree.setProperty (id::tuningSystemName, "12-TET", nullptr);

        return tree;
    }
};