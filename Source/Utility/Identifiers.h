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

    static const juce::Identifier MPE_ROUTING = "MPE_ROUTING";

    static const juce::Identifier MPE_SETTINGS = "MPE_SETTINGS";
    static const juce::Identifier MPE_CHANNEL  = "MPE_CHANNEL";
    static const juce::Identifier PRESSURE = "PRESSURE";
    static const juce::Identifier TIMBRE = "TIMBRE";
    static const juce::Identifier OUTPUT_ONE = "OUTPUT_ONE";
    static const juce::Identifier OUTPUT_TWO = "OUTPUT_TWO";
    static const juce::Identifier OUTPUT_THREE = "OUTPUT_THREE";
    static const juce::Identifier name = "name";
    static const juce::Identifier lowerBound = "lowerBound";
    static const juce::Identifier upperBound = "upperBound";
}