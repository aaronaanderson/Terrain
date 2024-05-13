#include "MainEditor.h"

MainEditor::MainEditor (MainProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), 
      state(processorRef.getState())
{

    setLookAndFeel (&lookAndFeel);

    setSize (1200, 800);
    setResizable (true, false);
    setResizeLimits (300, 200, 2400, 1600);
}
MainEditor::~MainEditor() 
{
    setLookAndFeel (nullptr);
}
void MainEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}
void MainEditor::resized() {}