#pragma once

#include "MainProcessor.h"
#include "Interface/TrajectoryPanel.h"
#include "Interface/TerrainPanel.h"
#include "Interface/ControlPanel.h"
#include "Interface/VisualiserPanel.h"

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
    juce::UndoManager& undoManager;
    GlobalTimer globalTimer;

    ti::TrajectoryPanel trajectoryPanel;    
    ti::TerrainPanel    terrainPanel;
    ti::ControlPanel    controlPanel;
    ti::VisualiserPanel visualiserPanel;

    TerrainLookAndFeel lookAndFeel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainEditor)
};