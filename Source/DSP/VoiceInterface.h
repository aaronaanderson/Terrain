#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace tp
{
struct VoiceInterface
{
    virtual void prepareToPlay (double newRate, int blockSize) = 0;
    virtual void setState (juce::ValueTree settingsBranch) = 0;
    virtual const float* getRawData() const = 0;
    virtual bool isVoiceCurrentlyActive() const = 0;
}; 
}