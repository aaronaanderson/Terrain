#pragma once

#include "MainProcessor.h"

class TerrainLookAndFeel : public juce::LookAndFeel_V4 {};

class MainEditor  : public juce::AudioProcessorEditor
{
public:
    explicit MainEditor (MainProcessor&);
    ~MainEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MainProcessor& processorRef; // Do NOT change order
    juce::ValueTree state;       // of processorRef and state xoxo

    TerrainLookAndFeel lookAndFeel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainEditor)
};