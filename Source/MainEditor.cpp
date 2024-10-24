#include "MainEditor.h"
#include "Interface/Renderer/glUtility.h"
MainEditor::MainEditor (MainProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), 
      state (processorRef.getState()), 
      undoManager (processorRef.getUndoManager())
{
    jassert (state.getType() == id::TERRAINSYNTH);
    
    trajectoryPanel = std::make_unique<ti::TrajectoryPanel> (state, undoManager, globalTimer, processorRef.getParameters()); 
    terrainPanel = std::make_unique<ti::TerrainPanel> (state, undoManager, globalTimer, processorRef.getParameters()); 
    controlPanel = std::make_unique<ti::ControlPanel> (state, undoManager, globalTimer, processorRef.getParameters());
    visualizerPanel = std::make_unique<ti::VisualizerPanel> (processorRef.getWaveTerrainSynthesizer(), processorRef.getParameters());
    
    addAndMakeVisible (trajectoryPanel.get());
    addAndMakeVisible (terrainPanel.get());
    addAndMakeVisible (controlPanel.get());
    addAndMakeVisible (visualizerPanel.get());

    state.addListener (this);
    setLookAndFeel (&lookAndFeel);
    getLookAndFeel().setDefaultLookAndFeel (&lookAndFeel);

    setResizable (true, false);
    setResizeLimits (756, 580, 2400, 1600);
    setSize (1200, 800);

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
    controlPanel->setBounds (b.removeFromBottom (controlPanelHeight));

    int quarterWidth = b.getWidth() / 4;
    auto trajectoryPanelBounds = b.removeFromLeft (quarterWidth);
    trajectoryPanel->setBounds (trajectoryPanelBounds);
    
    auto terrainPanelBounds = b.removeFromRight (quarterWidth);
    terrainPanel->setBounds (terrainPanelBounds);

    visualizerPanel->setBounds (b);
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
void MainEditor::valueTreeRedirected (juce::ValueTree& treeWhichHasBeenChanged) 
{
    juce::ignoreUnused (treeWhichHasBeenChanged); 
    resetInterface(); 
}
void MainEditor::resetInterface()
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