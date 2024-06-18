#include "MainEditor.h"
#include "Interface/Renderer/glUtility.h"
MainEditor::MainEditor (MainProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), 
      state(processorRef.getState()), 
      undoManager (processorRef.getUndoManager()),
      trajectoryPanel (state, undoManager, globalTimer, processorRef.getParameters()), 
      terrainPanel (state, undoManager, globalTimer, processorRef.getParameters()), 
      controlPanel (state, undoManager, globalTimer, processorRef.getParameters()),
      visualizerPanel (processorRef.getWaveTerrainSynthesizer(), processorRef.getParameters())
{
    jassert (state.getType() == id::TERRAINSYNTH);
    setLookAndFeel (&lookAndFeel);
    getLookAndFeel().setDefaultLookAndFeel (&lookAndFeel);

    setResizable (true, false);
    setResizeLimits (756, 580, 2400, 1600);
    setSize (1200, 800);
    addAndMakeVisible (trajectoryPanel);
    addAndMakeVisible (terrainPanel);
    addAndMakeVisible (controlPanel);
    addAndMakeVisible (visualizerPanel);

    setWantsKeyboardFocus (true);
}
MainEditor::~MainEditor() 
{
    setLookAndFeel (nullptr);
}
void MainEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}
void MainEditor::resized() 
{
    auto b = getLocalBounds();
    
    int controlPanelHeight = b.getHeight() / 5;
    controlPanel.setBounds (b.removeFromBottom (controlPanelHeight));

    int quarterWidth = b.getWidth() / 4;
    auto trajectoryPanelBounds = b.removeFromLeft (quarterWidth);
    trajectoryPanel.setBounds (trajectoryPanelBounds);
    
    auto terrainPanelBounds = b.removeFromRight (quarterWidth);
    terrainPanel.setBounds (terrainPanelBounds);

    visualizerPanel.setBounds (b);
}
bool MainEditor::keyPressed (const juce::KeyPress& key) 
{   
    if(key.getModifiers().isCommandDown() && (key.getKeyCode() == 'v' || key.getKeyCode() == 'V'))
    {
        valueTreeViewWindow = std::make_unique<ValueTreeViewWindow> (state);
        valueTreeViewWindow->setSize (600, 800);
        valueTreeViewWindow->addToDesktop ();
        valueTreeViewWindow->setVisible (true);
        return true;
    }
    return true;
}