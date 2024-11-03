#pragma once

#include "Visualizer.h"
#include "SettingsComponent.h"
#include "../Parameters.h"
#include "LookAndFeel.h"
#include "../Utility/Identifiers.h"
namespace ti
{
class CenterConsole : public juce::TabbedComponent
{
public:
    CenterConsole (tp::WaveTerrainSynthesizerStandard& wts, 
                   tp::WaveTerrainSynthesizerMPE& wtsmpe,
                   const tp::Parameters& p, 
                   juce::ValueTree settingsBranch, 
                   const juce::AudioProcessorValueTreeState& apvts)
      :  juce::TabbedComponent (juce::TabbedButtonBar::Orientation::TabsAtTop), 
         visualizer (wts, wtsmpe, p), 
         settingsComponent (settingsBranch, apvts)
    { 
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        auto c = laf->getBackgroundColour();
        addTab ("Visualizer", c, &visualizer, false);
        addTab ("Settings", c, &settingsComponent, false);
    }
private:
    Visualizer visualizer;
    SettingsComponent settingsComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CenterConsole)
};
}