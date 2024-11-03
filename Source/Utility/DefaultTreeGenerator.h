#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include "Identifiers.h"

struct MPERoutingTree
{
    static juce::ValueTree create()
    {
        juce::ValueTree tree (id::MPE_ROUTING);
        juce::ValueTree pressureTree (id::PRESSURE);
        pressureTree.addChild (juce::ValueTree (id::OUTPUT_ONE), -1, nullptr);
        pressureTree.addChild (juce::ValueTree (id::OUTPUT_TWO), -1, nullptr);
        pressureTree.addChild (juce::ValueTree (id::OUTPUT_THREE), -1, nullptr);
        juce::ValueTree timbreTree (id::TIMBRE);
        timbreTree.addChild (juce::ValueTree (id::OUTPUT_ONE), -1, nullptr);
        timbreTree.addChild (juce::ValueTree (id::OUTPUT_TWO), -1, nullptr);
        timbreTree.addChild (juce::ValueTree (id::OUTPUT_THREE), -1, nullptr);
        
        tree.addChild (pressureTree, -1, nullptr);
        tree.addChild (timbreTree, -1, nullptr);

        return tree;
    }
};
struct SettingsTree
{
    struct DefaultSettings
    {
        static constexpr float presetRandomizationScale = 0.2f;
        static constexpr int oversampling = 1;
        static constexpr float pitchBendRange = 2.0f;
        static constexpr bool noteOnOrContinuous = false;
        static constexpr bool mpeEnabled = false;
    };
    static juce::ValueTree create()
    {
        juce::ValueTree tree (id::PRESET_SETTINGS);
        tree.setProperty (id::presetRandomizationScale, DefaultSettings::presetRandomizationScale, nullptr);
        tree.setProperty (id::oversampling, DefaultSettings::oversampling, nullptr);
        tree.setProperty (id::pitchBendRange, DefaultSettings::pitchBendRange, nullptr);
        tree.setProperty (id::mpeEnabled, DefaultSettings::mpeEnabled, nullptr);
        // true = continuous
        tree.setProperty (id::noteOnOrContinuous, DefaultSettings::noteOnOrContinuous, nullptr);
        
        tree.addChild (MPERoutingTree::create(), -1, nullptr);
        return tree;
    }
};

struct EphemeralStateTree
{
    struct DefaultSettings
    {
        static constexpr bool tuningSystemConnected = false;
        static constexpr char* tuningSystemName = "12-TET";
    };
    static juce::ValueTree create()
    {
        juce::ValueTree tree (id::EPHEMERAL_STATE);
        tree.setProperty (id::tuningSystemConnected, DefaultSettings::tuningSystemConnected, nullptr);
        tree.setProperty (id::tuningSystemName, DefaultSettings::tuningSystemName, nullptr);

        return tree;
    }
};