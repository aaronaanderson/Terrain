#pragma once

#include "MainProcessor.h"
#include "Interface/TrajectoryPanel.h"
#include "Interface/TerrainPanel.h"
#include "Interface/ControlPanel.h"
#include "Interface/VisualiserPanel.h"

#include "Interface/ValueTreeView.h"
#include "Interface/LookAndFeel.h"
class MainEditor  : public juce::AudioProcessorEditor, 
                    private juce::ValueTree::Listener
{
public:
    explicit MainEditor (MainProcessor&);
    ~MainEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
private:
    MainProcessor& processorRef; // Do NOT change order
    juce::ValueTree& state;       // of processorRef and state xoxo
    juce::UndoManager& undoManager;
    GlobalTimer globalTimer;

    TerrainLookAndFeel lookAndFeel;

    std::unique_ptr<ti::TrajectoryPanel> trajectoryPanel;    
    std::unique_ptr<ti::TerrainPanel>    terrainPanel;
    std::unique_ptr<ti::ControlPanel>    controlPanel;
    std::unique_ptr<ti::VisualizerPanel> visualizerPanel;

    std::unique_ptr<ValueTreeViewWindow> valueTreeViewWindow;
    bool keyPressed (const juce::KeyPress& key) override;
    
    void valueTreeRedirected (juce::ValueTree& treeWhichHasBeenChanged) override 
    {
        juce::ignoreUnused (treeWhichHasBeenChanged); 
        resetInterface(); 
    }
    void resetInterface()
    {
        removeChildComponent (trajectoryPanel.get());
        removeChildComponent (terrainPanel.get());
        removeChildComponent (controlPanel.get());

        trajectoryPanel = std::make_unique<ti::TrajectoryPanel> (state, undoManager, globalTimer, processorRef.getParameters()); 
        terrainPanel = std::make_unique<ti::TerrainPanel> (state, undoManager, globalTimer, processorRef.getParameters()); 
        controlPanel = std::make_unique<ti::ControlPanel> (state, undoManager, globalTimer, processorRef.getParameters());

        addAndMakeVisible (trajectoryPanel.get());
        addAndMakeVisible (terrainPanel.get());
        addAndMakeVisible (controlPanel.get());

        resized(); repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainEditor)
};