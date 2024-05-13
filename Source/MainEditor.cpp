#include "MainEditor.h"

MainEditor::MainEditor (MainProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), 
      state(processorRef.getState()), 
      undoManager (processorRef.getUndoManager()),
      trajectoryPanel (state.getChildWithName (id::TRAJECTORIES), undoManager, globalTimer)
{
    jassert (state.getType() == id::TERRAINSYNTH);
    std::cout << state.toXmlString() << std::endl;
    setLookAndFeel (&lookAndFeel);

    setSize (1200, 800);
    setResizable (true, false);
    setResizeLimits (300, 200, 2400, 1600);

    addAndMakeVisible (trajectoryPanel);
    addAndMakeVisible (terrainPanel);
    addAndMakeVisible (controlPanel);
    addAndMakeVisible (visualiserPanel);

}
MainEditor::~MainEditor() 
{
    setLookAndFeel (nullptr);
}
void MainEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto b = getLocalBounds();
    
    if (b.getHeight() > 500)
    {
        controlPanel.setVisible (true);
        int controlPanelHeight = b.getHeight() / 6;
        controlPanel.setBounds (b.removeFromBottom (controlPanelHeight));
    } else {
        controlPanel.setVisible (false);
    }

    if (b.getWidth() > 600)
    {
        int quarterWidth = b.getWidth() / 4;
        auto trajectoryPanelBounds = b.removeFromLeft (quarterWidth);
        trajectoryPanel.setBounds (trajectoryPanelBounds);
        trajectoryPanel.setVisible (true);
    
        auto terrainPanelBounds = b.removeFromRight (quarterWidth);
        terrainPanel.setBounds (terrainPanelBounds);
        terrainPanel.setVisible (true);
    } else {
        trajectoryPanel.setVisible (false);
        terrainPanel.setVisible (false);
    }
    visualiserPanel.setBounds (b);
}
void MainEditor::resized() {}