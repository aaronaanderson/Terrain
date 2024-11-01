#pragma once 

#include "../DefaultTreeGenerator.h"

struct EphemeralState : private juce::Timer
{
    EphemeralState (MainProcessor& p)
      : processorRef (p)
    {
        state = EphemeralStateTree::create();
        startTimer (3000);
        timerCallback();
    }
    void timerCallback() override
    {
        state.setProperty (id::tuningSystemConnected, processorRef.getMTSConnectionStatus(), nullptr);
        state.setProperty (id::tuningSystemName, processorRef.getTuningSystemName(), nullptr);
    }
    juce::ValueTree getState() { return state; }
private:
    MainProcessor& processorRef;
    juce::ValueTree state;
};