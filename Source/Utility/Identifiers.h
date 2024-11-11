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
    static const juce::Identifier pressureCurve = "pressureCurve";
    static const juce::Identifier timbreCurve = "timbreCurve";
    static const juce::Identifier pressureSmoothing = "pressureSmoothing";
    static const juce::Identifier timbreSmoothing = "timbreSmoothing";
    static const juce::Identifier releaseSensitivity = "releaseSensitivity";
    static const juce::Identifier pitchBendEnabled = "pitchBendEnabled";
    static const juce::Identifier pitchBendDivisionOfOctave = "pitchBendDivisionOfOctave";

    static const juce::Identifier MPE_CHANNEL  = "MPE_CHANNEL";
    static const juce::Identifier PRESSURE = "PRESSURE";
    static const juce::Identifier TIMBRE = "TIMBRE";
    static const juce::Identifier OUTPUT_ONE = "OUTPUT_ONE";
    static const juce::Identifier OUTPUT_TWO = "OUTPUT_TWO";
    static const juce::Identifier OUTPUT_THREE = "OUTPUT_THREE";
    static const juce::Identifier OUTPUT_FOUR = "OUTPUT_FOUR";
    static const juce::Identifier OUTPUT_FIVE = "OUTPUT_FIVE";
    static const juce::Identifier OUTPUT_SIX = "OUTPUT_SIX";
    static const juce::Identifier name = "name";
    static const juce::Identifier handleOne = "handleOne";
    static const juce::Identifier handleTwo = "handleTwo";
    static const juce::Identifier curve = "curve";

    static const juce::Identifier VOICES_STATE = "VOICES_STATE";
    static const juce::Identifier VOICE_CHANNEL = "VOICE_CHANNEL";
    static const juce::Identifier voicePressure = "voicePressure";
    static const juce::Identifier voiceTimbre = "voiceTimbre";
    static const juce::Identifier voiceActive = "voiceActive";
}