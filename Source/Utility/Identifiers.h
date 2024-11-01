#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace id 
{
    static const juce::Identifier TERRAIN_SYNTH = "TERRAIN_SYNTH";

    static const juce::Identifier PRESET_SETTINGS = "PRESET_SETTINGS";
    static const juce::Identifier presetName = "presetName";
    static const juce::Identifier presetRandomizationScale = "presetRandomizationScale";
    static const juce::Identifier oversampling = "oversampling";
    static const juce::Identifier pitchBendRange = "pitchBendRange";
    static const juce::Identifier mpeEnabled = "mpeEnabled";
    static const juce::Identifier version = JucePlugin_VersionString;

    static const juce::Identifier noteOnOrContinuous = "noteOnOrContinuous";


    static const juce::Identifier EPHEMERAL_STATE = "EPHEMERAL_STATE";
    static const juce::Identifier tuningSystemName = "tuningSystemName";
    static const juce::Identifier tuningSystemConnected = "tuningSystemConnected";
}