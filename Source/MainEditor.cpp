#include "MainEditor.h"

MainEditor::MainEditor (MainProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), 
      state(processorRef.getState()), 
      undoManager (processorRef.getUndoManager()),
      trajectoryPanel (state.getChildWithName (id::TRAJECTORIES), undoManager, globalTimer, processorRef.getParameters())
{
    jassert (state.getType() == id::TERRAINSYNTH);
    setLookAndFeel (&lookAndFeel);

    setSize (1200, 800);
    setResizable (true, false);
    setResizeLimits (300, 200, 2400, 1600);

    addAndMakeVisible (trajectoryPanel);
    addAndMakeVisible (terrainPanel);
    addAndMakeVisible (controlPanel);
    addAndMakeVisible (visualiserPanel);

    setWantsKeyboardFocus (true);
    addKeyListener (this);

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
bool MainEditor::keyPressed (const juce::KeyPress& key,
                             juce::Component* originatingComponent) 
{
juce::ignoreUnused(originatingComponent);
        
    if(key.getModifiers().isCommandDown() && (key.getKeyCode() == 'v' || key.getKeyCode() == 'V'))
    {
        if (valueTreeViewWindow == nullptr)
            valueTreeViewWindow = std::make_unique<ValueTreeViewWindow> (state);
        valueTreeViewWindow->addToDesktop ();
        valueTreeViewWindow->setSize (400, 800);
        valueTreeViewWindow->setVisible (true);
        return true;
    }
    return true;
}