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
                   const juce::AudioProcessorValueTreeState& apvts, 
                   juce::ValueTree mpePresets)
      :  juce::TabbedComponent (juce::TabbedButtonBar::Orientation::TabsAtTop), 
         visualizer (wts, wtsmpe, p), 
         settingsComponent (settingsBranch, apvts, mpePresets)
    { 
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        auto c = laf->getBackgroundColour();
        addTab ("Visualizer", c, &visualizer, false);
        addTab ("Settings", c, &settingsComponent, false);
    }
    void setState (juce::ValueTree settingsBranch) { settingsComponent.setState (settingsBranch); }
private:
    Visualizer visualizer;
    SettingsComponent settingsComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CenterConsole)
};
}