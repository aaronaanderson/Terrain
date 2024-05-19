#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace id 
{
    static const juce::Identifier TERRAINSYNTH = "TERRAIN_SYNTH";
    
    static const juce::Identifier TRAJECTORIES = "TRAJECTORIES";
    static const juce::Identifier TRAJECTORY = "TRAJECTORY"; 
    static const juce::Identifier type = "type";
    static const juce::Identifier currentTrajectory = "currentTrajectory";

    static const juce::Identifier MODIFIERS = "MODIFIERS";
    static const juce::Identifier mod_A = "mod_A";
    static const juce::Identifier mod_B = "mod_B";
    static const juce::Identifier mod_C = "mod_C";
    static const juce::Identifier mod_D = "mod_D";

    static const juce::Identifier TRAJECTORY_VARIABLES = "TRAJECTORY_VARIABLES";
    static const juce::Identifier size = "size";
    static const juce::Identifier rotation = "rotation";
    static const juce::Identifier translation_x = "translation_x";
    static const juce::Identifier translation_y = "translation_y";

}