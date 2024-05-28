#pragma once

#include "Panel.h"
#include "Visualizer.h"
#include "../Parameters.h"
namespace ti
{
class VisualizerPanel : public Panel
{
public:
    VisualizerPanel (const tp::Parameters& parameters)
      : Panel ("Visualizer"), 
        visualizer (parameters)
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