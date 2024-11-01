#pragma once

#include "MainProcessor.h"
#include "Interface/TrajectoryPanel.h"
#include "Interface/TerrainPanel.h"
#include "Interface/ControlPanel.h"
#include "Interface/VisualiserPanel.h"
#include "Interface/Header.h"

#include "Interface/ValueTreeView.h"
#include "Interface/LookAndFeel.h"

#include "DefaultTreeGenerator.h"

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

        std::cout << state.toXmlString() << std::endl;
    }
    juce::ValueTree getState() { return state; }
private:
    MainProcessor& processorRef;
    juce::ValueTree state;
};
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
    EphemeralState ephemeralState;

    TerrainLookAndFeel lookAndFeel;

    std::unique_ptr<ti::TrajectoryPanel> trajectoryPanel;    
    std::unique_ptr<ti::TerrainPanel>    terrainPanel;
    std::unique_ptr<ti::ControlPanel>    controlPanel;
    std::unique_ptr<ti::VisualizerPanel> visualizerPanel;
    std::unique_ptr<ti::Header>          header;
    std::unique_ptr<ValueTreeViewWindow> valueTreeViewWindow;
    
    bool keyPressed (const juce::KeyPress& key) override;
    void valueTreeRedirected (juce::ValueTree& treeWhichHasBeenChanged) override;
    void resetInterface();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainEditor)
};