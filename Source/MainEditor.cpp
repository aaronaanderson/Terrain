#include "MainEditor.h"
#include "Interface/Renderer/glUtility.h"
MainEditor::MainEditor (MainProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), 
      state (processorRef.getState())
{
    jassert (state.getType() == id::TERRAIN_SYNTH);
    
    getTopLevelComponent()->setName ("Terrain " + id::version.toString());

    trajectoryPanel = std::make_unique<ti::TrajectoryPanel> (processorRef.getValueTreeState()); 
    terrainPanel = std::make_unique<ti::TerrainPanel> (processorRef.getValueTreeState()); 
    controlPanel = std::make_unique<ti::ControlPanel> (processorRef.getValueTreeState());
    visualizerPanel = std::make_unique<ti::VisualizerPanel> (processorRef.getWaveTerrainSynthesizer(), 
                                                             processorRef.getCastedParameters());
    header = std::make_unique<ti::Header> (processorRef.getPresetManager(), 
                                           processorRef.getState().getChildWithName (id::PRESET_SETTINGS));

    addAndMakeVisible (trajectoryPanel.get());
    addAndMakeVisible (terrainPanel.get());
    addAndMakeVisible (controlPanel.get());
    addAndMakeVisible (visualizerPanel.get());
    addAndMakeVisible (header.get());

    state.addListener (this);
    setLookAndFeel (&lookAndFeel);
    getLookAndFeel().setDefaultLookAndFeel (&lookAndFeel);

    setResizable (true, false);
    setResizeLimits (756, 620, 2400, 1600);
    setSize (1200, 800);
}
MainEditor::~MainEditor() 
{
    setLookAndFeel (nullptr);
    state.removeListener (this);
}
void MainEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}
void MainEditor::resized() 
{
    auto b = getLocalBounds();

    header->setBounds (b.removeFromTop (40));
    
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
    removeChildComponent (header.get());
    
    trajectoryPanel = std::make_unique<ti::TrajectoryPanel> (processorRef.getValueTreeState()); 
    terrainPanel = std::make_unique<ti::TerrainPanel> (processorRef.getValueTreeState()); 
    controlPanel = std::make_unique<ti::ControlPanel> (processorRef.getValueTreeState());
    header = std::make_unique<ti::Header> (processorRef.getPresetManager(), 
                                           processorRef.getState().getChildWithName (id::PRESET_SETTINGS));

    addAndMakeVisible (trajectoryPanel.get());
    addAndMakeVisible (terrainPanel.get());
    addAndMakeVisible (controlPanel.get());
    addAndMakeVisible (header.get());
    resized(); repaint();
}