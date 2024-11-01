#pragma once

#include "Visualizer.h"
#include "../Parameters.h"
#include "LookAndFeel.h"

namespace ti
{
class CenterConsole : public juce::TabbedComponent
{
public:
    CenterConsole (tp::WaveTerrainSynthesizer& wts, const tp::Parameters& p)
      :  juce::TabbedComponent (juce::TabbedButtonBar::Orientation::TabsAtTop), 
         visualizer (wts, p)
    { 
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        auto c = laf->getBaseColour();
        addTab ("Visualizer", c, &visualizer, false);
    }
private:
    Visualizer visualizer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CenterConsole)
};
}