#pragma once

#include "Panel.h"
#include "Visualizer.h"
#include "../Parameters.h"
namespace ti
{
class VisualizerPanel : public Panel
{
public:
    VisualizerPanel (tp::WaveTerrainSynthesizer& wts, const tp::Parameters& parameters)
      : Panel ("Visualizer"), 
        visualizer (wts, parameters)
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