#pragma once

#include <juce_core/juce_core.h>

struct Version
{
    Version (juce::String version)
    {
        major = version[0];
        minor = version[2];
        patch = version[4];
    };
    int major;
    int minor;
    int patch;
};