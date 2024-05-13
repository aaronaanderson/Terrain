#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace id 
{
    static const juce::Identifier TERRAINSYNTH = "TERRAINSYNTH";
}
struct DefaultTree
{
    static const juce::ValueTree create()
    {
        auto tree = juce::ValueTree (id::TERRAINSYNTH);
    }
};