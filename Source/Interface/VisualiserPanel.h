#pragma once

#include "Panel.h"
#include "Visualiser.h"
namespace ti
{
class VisualizerPanel : public Panel
{
public:
    VisualizerPanel ()
      : Panel ("Visualizer")
    {
        addAndMakeVisible (visualizer);
    }
    void resized() override 
    {
        auto b = getAdjustedBounds();
        visualizer.setBounds (b);
    }
private:
    Visualizer visualizer;
};
}