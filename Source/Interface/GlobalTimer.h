#pragma once

#include <juce_events/juce_events.h>

struct GlobalTimer : private juce::Timer
{
    GlobalTimer() { startTimerHz (60); }

    struct Listener
    {
        virtual void onTimerCallback() = 0;
    };
    void addListener (Listener& l) { listeners.add (&l); }
private:
    juce::Array<Listener*> listeners;

    void timerCallback() override
    {
        for (auto l : listeners)
            l->onTimerCallback();
    }
};