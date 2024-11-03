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
    RoutingComponent (juce::ValueTree MPERouting, 
                      const juce::Identifier& MPEChannel)
      : mpeRouting (MPERouting), 
        draggableAssignerOne (MPERouting, MPEChannel, id::OUTPUT_ONE), 
        draggableAssignerTwo (MPERouting, MPEChannel, id::OUTPUT_TWO), 
        draggableAssignerThree (MPERouting, MPEChannel, id::OUTPUT_THREE)
    {
        jassert (mpeRouting.getType() == id::MPE_ROUTING);
        destinationLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (destinationLabel);
        addAndMakeVisible (draggableAssignerOne);
        addAndMakeVisible (draggableAssignerTwo);
        addAndMakeVisible (draggableAssignerThree);
    }
    void resized() override
    {
        juce::Rectangle<int> cdBounds {200, 20};
        destinationLabel.setBounds (cdBounds);
        draggableAssignerOne.setBounds (cdBounds.withPosition (0, 20));
        draggableAssignerTwo.setBounds (cdBounds.withPosition (0, 48));
        draggableAssignerThree.setBounds (cdBounds.withPosition (0, 76));
    }
    struct DraggableAssigner : public juce::DragAndDropContainer, 
                               public juce::Component
    {
        DraggableAssigner (juce::ValueTree MPERouting, 
                           const juce::Identifier& MPEChannel, 
                           const juce::Identifier& outChannel)
          : mpeRouting (MPERouting), 
            mpeChannel (MPEChannel), 
            outputChannel (outChannel)
        {}
        void paint (juce::Graphics& g) override
        {
           auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
           g.setColour (laf->getBackgroundDark());
           g.drawRect (getLocalBounds().toFloat(), 2.0f);
           
           g.setColour (juce::Colours::white);
           g.drawText (name, 
                       getLocalBounds().toFloat(), 
                       juce::Justification::centred); 
        }
        void mouseDrag (const juce::MouseEvent& event) override 
        {
            juce::ignoreUnused (event);
            startDragging ("MPE Source", this, juce::ScaledImage(), true);
        }
        juce::ValueTree getMPEChannelRouting() { return mpeRouting.getChildWithName (mpeChannel)
                                                                  .getChildWithName (outputChannel); }
        void setLabel (juce::String label) { name = label; repaint(); }
    private:
        juce::ValueTree mpeRouting;
        const juce::Identifier& mpeChannel;
        const juce::Identifier& outputChannel;
        juce::String name {"Drag to assign"};
    };
private:
    juce::ValueTree mpeRouting;
    juce::Label destinationLabel {"destination", "Destination"};
    DraggableAssigner draggableAssignerOne;
    DraggableAssigner draggableAssignerTwo;
    DraggableAssigner draggableAssignerThree;
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
        routingComponent (mpeRouting, mpeChannel)
    {
        // jassert (mpeSettings.getType() == id::MPE_SETTINGS);
        jassert (mpeRouting.getType() == id::MPE_ROUTING);
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
                                   id::PRESSURE),
         timbreChannelComponent (settingsBranch.getChildWithName (id::MPE_ROUTING), 
                                 juce::ValueTree(), // todo: make MPE_SETTINGS tree
                                 "Timbre", 
                                 id::TIMBRE)
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