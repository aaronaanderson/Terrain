#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "LookAndFeel.h"
namespace ti
{
struct HeaderLabel : public juce::Component
{
    HeaderLabel (juce::String name)
    {
        label.setJustificationType (juce::Justification::centred);
        label.setText (name, juce::dontSendNotification);
        addAndMakeVisible (label);
    }
    void paint (juce::Graphics& g) override 
    {
        auto b = getLocalBounds();
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        g.setColour (laf->getBackgroundDark());
        g.drawRect (b.toFloat().toFloat(), 2.0f);
    }
    void resized() override 
    {
        label.setBounds (getLocalBounds());
    }
private:
    juce::Label label;
};

struct MPECurve : public juce::Component
{
    MPECurve (juce::ValueTree MPESettings, 
              const juce::Identifier& MPEChannel)
      : mpeSettings (MPESettings), 
        mpeChannel (MPEChannel) 
    {

    }
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds();
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        g.setColour (laf->getBackgroundDark());
        g.fillRect (b.toFloat());

        g.setColour (laf->getAccentColour());
        g.drawLine (0.0f, 
                    static_cast<float> (getHeight()), 
                    static_cast<float> (getWidth()), 
                    0.0f, 
                    4.0f);
    }
private:
    juce::ValueTree mpeSettings;
    const juce::Identifier& mpeChannel;
};
struct RoutingComponent : public juce::Component
{
    RoutingComponent (juce::ValueTree MPESettings)
      :  mpeSettings (MPESettings)
    {
        destinationLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (destinationLabel);
        addAndMakeVisible (channelDestination);
    }
    void resized() override
    {
        juce::Rectangle<int> cdBounds {200, 20};
        destinationLabel.setBounds (cdBounds);
        channelDestination.setBounds (cdBounds.withPosition (0, 20));
    }
private:
    juce::ValueTree mpeSettings;
    juce::ComboBox channelDestination;
    juce::Label destinationLabel {"destination", "Destination"};
};
struct MPEChannelComponent : public juce::Component 
{
    MPEChannelComponent (juce::ValueTree MPERouting, 
                         juce::ValueTree MPESettings,
                         juce::String whichChannel, 
                         const juce::Identifier& mpeChannel)
      : mpeRouting (MPERouting), 
        mpeSettings (MPESettings), 
        mpeCurveComponent (MPESettings, mpeChannel),  
        routingComponent (MPESettings)
    {
        channelNameLabel.setJustificationType (juce::Justification::left);
        channelNameLabel.setText (whichChannel, juce::dontSendNotification);
        addAndMakeVisible (channelNameLabel);
        addAndMakeVisible (mpeCurveComponent);
        addAndMakeVisible (routingComponent);
    }
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        auto b = getLocalBounds();
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        g.setColour (laf->getBackgroundDark());
        g.drawRect (b.removeFromTop(labelHeight).toFloat(), 2.0f);
        g.drawRect (b.toFloat(), 2.0f);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        channelNameLabel.setBounds (b.removeFromTop (labelHeight));
        mpeCurveComponent.setBounds (b.removeFromLeft (120).reduced (4));
        routingComponent.setBounds (b);
    }
private:
    juce::ValueTree mpeRouting;
    juce::ValueTree mpeSettings;
    juce::Label channelNameLabel;
    MPECurve mpeCurveComponent;
    RoutingComponent routingComponent;
    const int labelHeight = 20;
};

class SettingsComponent : public juce::Component
{
public:
    SettingsComponent (juce::ValueTree settingsBranch)
      :  settings (settingsBranch), 
         mpeHeader ("MPE"), 
         pressureChannelComponent (settingsBranch.getChildWithName (id::MPE_ROUTING), 
                                   juce::ValueTree(), // todo: make MPE_SETTINGS tree
                                   "Pressure", 
                                   id::pressure),
         timbreChannelComponent (settingsBranch.getChildWithName (id::MPE_ROUTING), 
                                   juce::ValueTree(), // todo: make MPE_SETTINGS tree
                                   "Timbre", 
                                   id::timbre)
    {
        jassert (settings.getType() == id::PRESET_SETTINGS);
        addAndMakeVisible (mpeHeader);
        addAndMakeVisible (pressureChannelComponent);
        addAndMakeVisible (timbreChannelComponent);
    }
    void resized() override
    {
        auto b = getLocalBounds();

        mpeHeader.setBounds (b.removeFromTop (20));
        pressureChannelComponent.setBounds (b.removeFromTop (120));
        timbreChannelComponent.setBounds (b.removeFromTop (120));
    }
private:
    juce::ValueTree settings;
    HeaderLabel mpeHeader;
    MPEChannelComponent pressureChannelComponent;
    MPEChannelComponent timbreChannelComponent;
};
}