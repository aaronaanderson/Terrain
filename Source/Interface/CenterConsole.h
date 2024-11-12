#pragma once

#include "Visualizer.h"
#include "SettingsComponent.h"
#include "../Parameters.h"
#include "LookAndFeel.h"
#include "../Utility/Identifiers.h"
namespace ti
{
class SpecialTabComponent : public juce::TabbedComponent
{
public:
    SpecialTabComponent (juce::TabbedButtonBar::Orientation o)
      : juce::TabbedComponent (o)
    {}
    void currentTabChanged (int newCurrentTabIndex, const juce::String& newCurrentTabName) override
    {
        if (onTabChange != nullptr) onTabChange (newCurrentTabIndex, newCurrentTabName);
        juce::TabbedComponent::currentTabChanged(newCurrentTabIndex, newCurrentTabName);
    }

    std::function<void(int, const juce::String&)> onTabChange = nullptr;
};
class CenterConsole : public juce::TabbedComponent
{
public:
    CenterConsole (tp::WaveTerrainSynthesizerStandard& wts, 
                   tp::WaveTerrainSynthesizerMPE& wtsmpe,
                   const tp::Parameters& p, 
                   juce::ValueTree settingsBranch, 
                   const juce::AudioProcessorValueTreeState& apvts, 
                   juce::ValueTree& mpeSettings, 
                   juce::ValueTree voicesState)
      :  juce::TabbedComponent (juce::TabbedButtonBar::Orientation::TabsAtTop),
         visualizer (wts, wtsmpe, p, settingsBranch, voicesState, apvts),
         settingsComponent (settingsBranch, apvts, mpeSettings)
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

    struct TestComponent : public juce::Component {};
    TestComponent testComponent;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CenterConsole)
};
}