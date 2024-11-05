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
        jassert (mpeSettings.getType() == id::MPE_SETTINGS);

        curveFactorSlider.setRange ({0.125, 8.0}, 0.0);
        curveFactorSlider.setSkewFactorFromMidPoint (1.0);
        curveFactorSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 20, 20);
        float value = 1.0f;
        if (mpeChannel == id::PRESSURE) value = mpeSettings.getProperty (id::pressureCurve);
        else if (mpeChannel == id::TIMBRE) value = mpeSettings.getProperty (id::timbreCurve);
        curveFactorSlider.setValue (value);
        curveFactorSlider.setDoubleClickReturnValue (true, 1.0);
        curveFactorSlider.onValueChange = [&]() 
        { 
            if (MPEChannel == id::PRESSURE)
                mpeSettings.setProperty (id::pressureCurve, curveFactorSlider.getValue(), nullptr);
            else if (MPEChannel == id::TIMBRE)
                mpeSettings.setProperty (id::timbreCurve, curveFactorSlider.getValue(), nullptr);

            repaint(); 
        };
        addAndMakeVisible (curveFactorSlider);
    }
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds();
        b.removeFromBottom (static_cast<int> (b.getHeight() * 0.2f));
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        g.setColour (laf->getBackgroundDark());
        g.fillRect (b.toFloat());

        g.setColour (laf->getAccentColour());

        juce::Path curve;
        float curveThicc = 4.0f;
        curve.startNewSubPath ({0.0f, static_cast<float> (b.getHeight())});
        for (int i = 0; i < b.getWidth(); i++)
        {
            auto normalX = juce::jmap (static_cast<float> (i), 0.0f, static_cast<float> (b.getWidth()) - 1.0f, 0.0f, 1.0f);
            auto normalY = static_cast<float> (std::pow (normalX, 1.0f / curveFactorSlider.getValue()));

            juce::Point<float> nextPoint = {juce::jmap (normalX, curveThicc * 0.5f, static_cast<float> (b.getWidth())), 
                                            juce::jmap (normalY, static_cast<float> (b.getHeight()), 0.0f)};
            curve.lineTo (nextPoint); 
        }  
        g.strokePath (curve, juce::PathStrokeType (curveThicc));
    }
    void resized()
    {
        auto b = getLocalBounds();
        b.removeFromTop (static_cast<int> (b.getHeight() * 0.8f));
        curveFactorSlider.setBounds (b);
    }
private:
    juce::ValueTree mpeSettings;
    const juce::Identifier& mpeChannel;
    juce::Slider curveFactorSlider;
};
struct PressureSmoothingComponent : public juce::Component
{
    PressureSmoothingComponent (juce::ValueTree& MPESettings)
      : mpeSettings (MPESettings)
    {
        jassert (mpeSettings.getType() == id::MPE_SETTINGS);
        addAndMakeVisible (label);
        smoothingSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 60, 18);
        smoothingSlider.setNumDecimalPlacesToDisplay (1);
        smoothingSlider.setRange ({2.5, 1280.0}, 0.0);
        smoothingSlider.setSkewFactorFromMidPoint (80.0);
        smoothingSlider.setValue (mpeSettings.getProperty (id::pressureSmoothing), juce::dontSendNotification);
        smoothingSlider.onValueChange = [&]()
            {
                mpeSettings.setProperty (id::pressureSmoothing, smoothingSlider.getValue(), nullptr);
            };
        addAndMakeVisible (smoothingSlider);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromLeft (150));
        smoothingSlider.setBounds (b.removeFromLeft (250));
    }
private:
    juce::ValueTree mpeSettings;
    juce::Label label {"pSMooth", "Pressure Smoothing (ms)"};
    juce::Slider smoothingSlider;
};
struct TimbreSmoothingComponent : public juce::Component
{
    TimbreSmoothingComponent (juce::ValueTree& MPESettings)
      : mpeSettings (MPESettings)
    {
        jassert (mpeSettings.getType() == id::MPE_SETTINGS);
        addAndMakeVisible (label);
        smoothingSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 60, 18);
        smoothingSlider.setNumDecimalPlacesToDisplay (1);
        smoothingSlider.setRange ({2.5, 1280.0}, 0.0);
        smoothingSlider.setSkewFactorFromMidPoint (80.0);
        smoothingSlider.setValue (mpeSettings.getProperty (id::timbreSmoothing), juce::dontSendNotification);
        smoothingSlider.onValueChange = [&]()
            {
                mpeSettings.setProperty (id::timbreSmoothing, smoothingSlider.getValue(), nullptr);
            };
        addAndMakeVisible (smoothingSlider);
    }
    void paint (juce::Graphics& g) override 
    {
        auto bounds = getLocalBounds();
        auto b = getLocalBounds();
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        g.setColour (laf->getBackgroundDark());
        g.drawRect (b.toFloat(), 2.0f);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromLeft (150));
        smoothingSlider.setBounds (b.removeFromLeft (250));
    }
private:
    juce::ValueTree mpeSettings;
    juce::Label label {"pSMooth", "Timbre Smoothing (ms)"};
    juce::Slider smoothingSlider;
};
struct DraggableAssigner : public juce::DragAndDropContainer, 
                           public juce::Component
{
    DraggableAssigner (juce::ValueTree MPERouting, 
                       const juce::Identifier& MPEChannel, 
                       const juce::Identifier& outChannel, 
                       const juce::AudioProcessorValueTreeState& apvts)
      : mpeRouting (MPERouting), 
        mpeChannel (MPEChannel), 
        outputChannel (outChannel), 
        valueTreeState (apvts)
    {
        auto paramID = mpeRouting.getChildWithName (mpeChannel)
                                 .getChildWithName (outputChannel)
                                 .getProperty (id::name).toString();
        if (paramID != "") name = apvts.getParameter (paramID)->getName (20);
    }
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
    void mouseUp (const juce::MouseEvent& event) override 
    {
        if(event.mods.isRightButtonDown())
        {
            juce::PopupMenu m;
            m.addItem(1, "Unassign");
            m.showMenuAsync (juce::PopupMenu::Options(), [&](int result)
            {
                if(result == 1)
                {
                    mpeRouting.getChildWithName (mpeChannel)
                              .getChildWithName (outputChannel)
                              .setProperty (id::name, "", nullptr);
                    name = "Drag to assign";
                    repaint();
                }  
            });
        }
    }
    juce::ValueTree getMPEChannelRouting() { return mpeRouting.getChildWithName (mpeChannel)
                                                              .getChildWithName (outputChannel); }
    void setLabel (juce::String label) { name = label; repaint(); }
    void setState (juce::ValueTree routingBranch) { mpeRouting = routingBranch; }
private:
    juce::ValueTree mpeRouting;
    const juce::Identifier& mpeChannel;
    const juce::Identifier& outputChannel;
    const juce::AudioProcessorValueTreeState& valueTreeState;
    juce::String name {"Drag to assign"};
};
struct RoutingOutputLane : public juce::Component
{
    RoutingOutputLane (juce::ValueTree MPERouting, 
                      const juce::Identifier& MPEChannelIdentifier, /*Pressure or Timbre*/
                      const juce::Identifier& outputChannelIdentifier,
                      const juce::AudioProcessorValueTreeState& apvts)
      : mpeRouting (MPERouting),
        mpeChannelIdentifier (MPEChannelIdentifier),
        mpeChannel (mpeRouting.getChildWithName (mpeChannelIdentifier)),
        outChannelIdentifier (outputChannelIdentifier),
        draggableAssigner (mpeRouting, mpeChannelIdentifier, outChannelIdentifier, apvts)
    {
        jassert (mpeRouting.getType() == id::MPE_ROUTING);
        addAndMakeVisible (draggableAssigner);
        
        range.onValueChange = [&]()
            {
                mpeChannel.getChildWithName (id::OUTPUT_ONE).setProperty (id::upperBound, range.getMaxValue(), nullptr);
                mpeChannel.getChildWithName (id::OUTPUT_ONE).setProperty (id::lowerBound, range.getMinValue(), nullptr);
            };
        range.setSliderStyle (juce::Slider::SliderStyle::TwoValueHorizontal);
        range.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 20, 20);
        range.setRange ({0.0f, 1.0f}, 0.0);
        range.setMinAndMaxValues (mpeChannel.getChildWithName (id::OUTPUT_ONE)
                                               .getProperty (id::lowerBound),
                                     mpeChannel.getChildWithName (id::OUTPUT_ONE)
                                               .getProperty (id::upperBound), 
                                     juce::dontSendNotification);
        addAndMakeVisible (range);
        invertToggle.setToggleState (mpeChannel.getChildWithName (id::OUTPUT_ONE)
                                                  .getProperty (id::invertRange), juce::dontSendNotification);
        invertToggle.onClick = [&]()
            {
                mpeChannel.getChildWithName (id::OUTPUT_ONE)
                          .setProperty (id::invertRange, invertToggle.getToggleState(), nullptr);

            };
        addAndMakeVisible (invertToggle);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        auto height = b.getHeight();
        juce::Rectangle<int> cdBounds {200, height};
        juce::Rectangle<int> rangeBounds {200, height};
        juce::Rectangle<int> toggleBounds {30, height};

        draggableAssigner.setBounds (cdBounds.withPosition (0, 0));
        invertToggle.setBounds (toggleBounds.withPosition (220, 0));
        range.setBounds (rangeBounds.withPosition (255, 0));
    }
private:
    juce::ValueTree mpeRouting;
    const juce::Identifier& mpeChannelIdentifier;
    const juce::Identifier& outChannelIdentifier;
    juce::ValueTree mpeChannel;
    DraggableAssigner draggableAssigner;
    juce::ToggleButton invertToggle;
    juce::Slider range;
};
struct RoutingComponent : public juce::Component
{
    RoutingComponent (juce::ValueTree MPERouting, 
                      const juce::Identifier& MPEChannel, 
                      const juce::AudioProcessorValueTreeState& apvts)
      : mpeRouting (MPERouting), 
        mpeChannel (mpeRouting.getChildWithName (MPEChannel)),
        mpeChannelIdentifier (MPEChannel),
        laneOne (mpeRouting, mpeChannelIdentifier, id::OUTPUT_ONE, apvts),
        laneTwo (mpeRouting, mpeChannelIdentifier, id::OUTPUT_TWO, apvts),
        laneThree (mpeRouting, mpeChannelIdentifier, id::OUTPUT_THREE, apvts)
    {
        jassert (mpeRouting.getType() == id::MPE_ROUTING);
        jassert (mpeChannel.getType() == MPEChannel);
        destinationLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (destinationLabel);
        
        rangeLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (rangeLabel);

        invertLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (invertLabel);

        addAndMakeVisible (laneOne);
        addAndMakeVisible (laneTwo);
        addAndMakeVisible (laneThree);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        juce::Rectangle<int> cdBounds {200, 20};
        juce::Rectangle<int> rangeBounds {200, 22};
        juce::Rectangle<int> toggleBounds {30, 22};
        destinationLabel.setBounds (cdBounds);
        invertLabel.setBounds (toggleBounds.withPosition (220, 0));
        rangeLabel.setBounds (cdBounds.withPosition (255, 0));
        
        b.removeFromTop (20);
        laneOne.setBounds (b.removeFromTop (21));
        
        b.removeFromTop (4);
        laneTwo.setBounds (b.removeFromTop (21));

        b.removeFromTop (4);
        laneThree.setBounds (b.removeFromTop (21));
    }
private:
    juce::ValueTree mpeRouting;
    const juce::Identifier& mpeChannelIdentifier;
    juce::ValueTree mpeChannel;
    juce::Label destinationLabel {"destination", "Destination"};
    juce::Label rangeLabel = {"RangeLabel", "Range"};
    juce::Label invertLabel = {"InvertLabel", "Inv."};
    
    RoutingOutputLane laneOne, laneTwo, laneThree;
};
struct MPESaveComponent : public juce::Component
{
public:
    MPESaveComponent (juce::ValueTree& MPESettings)
      : mpeSettings (MPESettings)
    {
        jassert (mpeSettings.getType() == id::MPE_SETTINGS);
        saveButton.onClick = [&]() { savePreset(); };
        addAndMakeVisible (saveButton);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        saveButton.setBounds (b.removeFromLeft (100));
    }
private:
    juce::ValueTree mpeSettings;
    juce::TextButton saveButton {"Save MPE Settings"};

    void savePreset()
    {
        auto mpeFolder = getMPEPresetFolder();
        auto xml = mpeSettings.createXml();
        auto file = getMPEPresetFolder().getChildFile ("MPESettings.xml");
        if (!file.existsAsFile()) file.setCreationTime (juce::Time::getCurrentTime());
        xml->writeTo (file);
    }
    juce::File getMPEPresetFolder()
    {
	    auto presetFolder = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory);
	
#ifdef JUCE_MAC
	    presetFolder = presetFolder.getChildFile("Audio").getChildFile("Presets");
#endif
	    presetFolder = presetFolder.getChildFile("Aaron Anderson").getChildFile("Terrain"); // "Imogen" is the name of my plugin
	    presetFolder = presetFolder.getChildFile ("MPEPresets");
        auto result = presetFolder.createDirectory();

	    return presetFolder;
    }
};
struct MPEChannelComponent : public juce::Component 
{
    MPEChannelComponent (juce::ValueTree MPERouting, 
                         juce::ValueTree& MPESettings,
                         const juce::AudioProcessorValueTreeState& apvts,
                         juce::String whichChannel, 
                         const juce::Identifier& mpeChannel)
      : mpeRouting (MPERouting), 
        mpeSettings (MPESettings), 
        mpeCurveComponent (MPESettings, mpeChannel),  
        routingComponent (mpeRouting, mpeChannel, apvts)
    {
        (mpeSettings.getType() == id::MPE_SETTINGS);
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
    juce::ValueTree& mpeSettings;
    juce::Label channelNameLabel;
    MPECurve mpeCurveComponent;
    RoutingComponent routingComponent;
    const int labelHeight = 20;
};
struct PitchBendSettingsComponent : public juce::Component
{
    PitchBendSettingsComponent (juce::ValueTree MPESettings)
      : mpeSettings (MPESettings)
    {
        jassert (mpeSettings.getType() == id::MPE_SETTINGS);

        addAndMakeVisible (pitchBendEnabledLabel);
        addAndMakeVisible (pitchBendEnabled);
        addAndMakeVisible (divisionOfOctaveLabel);
        divisionOfOctave.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 40, 20);
        divisionOfOctave.setRange (1.0, 48.0, 1.0);
        addAndMakeVisible (divisionOfOctave);
    }
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        auto b = getLocalBounds();
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        g.setColour (laf->getBackgroundDark());
        g.drawRect (b.toFloat(), 2.0f);
    }
    void resized()
    {
        auto b = getLocalBounds();
        pitchBendEnabled.setToggleState (mpeSettings.getProperty (id::pitchBendEnabled), juce::dontSendNotification);
        pitchBendEnabled.onStateChange = [&]()
            {
                mpeSettings.setProperty (id::pitchBendEnabled, pitchBendEnabled.getToggleState(), nullptr);
            };
        pitchBendEnabled.setBounds (b.removeFromLeft (22));
        pitchBendEnabledLabel.setBounds (b.removeFromLeft (150));

        divisionOfOctave.setValue (mpeSettings.getProperty (id::pitchBendDivisionOfOctave), juce::dontSendNotification);
        divisionOfOctave.onValueChange = [&]()
            {
                mpeSettings.setProperty (id::pitchBendDivisionOfOctave, (int)divisionOfOctave.getValue(), nullptr);
            };
        divisionOfOctave.setBounds (b.removeFromLeft (228));
        divisionOfOctaveLabel.setBounds (b.removeFromLeft (200));
    }
private:
    juce::ValueTree mpeSettings;
    juce::Label pitchBendEnabledLabel {"pbel", "Pitch Bend Enabled"};
    juce::ToggleButton pitchBendEnabled;
    juce::Label divisionOfOctaveLabel {"dool", "Pitch Bend Division of Octave"};
    juce::Slider divisionOfOctave;
};
class SettingsComponent : public juce::Component
{
public:
    SettingsComponent (juce::ValueTree settingsBranch, 
                       const juce::AudioProcessorValueTreeState& apvts, 
                       juce::ValueTree& MPESettings)
      :  valueTreeState (apvts),
         settings (settingsBranch), 
         mpeSettings (MPESettings),
         mpeHeader ("MPE"), 
         saveComponent (MPESettings),
         pressureSmoothingComponent (MPESettings),
         timbreSmoothingComponent (MPESettings), 
         pitchBendComponent (MPESettings)
    {
        jassert (settings.getType() == id::PRESET_SETTINGS);

        pressureChannelComponent = std::make_unique<MPEChannelComponent> (settings.getChildWithName (id::MPE_ROUTING), 
                                                                          mpeSettings,
                                                                          valueTreeState,
                                                                          "Pressure", 
                                                                          id::PRESSURE);  
        timbreChannelComponent = std::make_unique<MPEChannelComponent> (settings.getChildWithName (id::MPE_ROUTING), 
                                                                        mpeSettings,
                                                                        valueTreeState,
                                                                        "Timbre", 
                                                                        id::TIMBRE);
        addAndMakeVisible (saveComponent);
        addAndMakeVisible (pressureChannelComponent.get());
        addAndMakeVisible (pressureSmoothingComponent);
        addAndMakeVisible (timbreChannelComponent.get());
        addAndMakeVisible (timbreSmoothingComponent);
        addAndMakeVisible (pitchBendComponent);
    }
    void resized() override
    {
        auto b = getLocalBounds();

        mpeHeader.setBounds (b.removeFromTop (20));
        saveComponent.setBounds (b.removeFromTop (20));
        pressureChannelComponent->setBounds (b.removeFromTop (120));
        pressureSmoothingComponent.setBounds (b.removeFromTop (20));
        timbreChannelComponent->setBounds (b.removeFromTop (120));
        timbreSmoothingComponent.setBounds (b.removeFromTop (24));
        pitchBendComponent.setBounds (b.removeFromTop (24));
    }
    void setState (juce::ValueTree settingsBranch)
    {
        settings = settingsBranch;
        resetChannelComponents();
    }
private:
    const juce::AudioProcessorValueTreeState& valueTreeState;
    juce::ValueTree& mpeSettings;
    juce::ValueTree settings;
    HeaderLabel mpeHeader;
    MPESaveComponent saveComponent;
    std::unique_ptr<MPEChannelComponent> pressureChannelComponent;
    PressureSmoothingComponent pressureSmoothingComponent;
    std::unique_ptr<MPEChannelComponent> timbreChannelComponent;
    TimbreSmoothingComponent timbreSmoothingComponent;
    PitchBendSettingsComponent pitchBendComponent;

    void resetChannelComponents()
    {
        removeChildComponent (pressureChannelComponent.get());
        removeChildComponent (timbreChannelComponent.get());
        pressureChannelComponent = std::make_unique<MPEChannelComponent> (settings.getChildWithName (id::MPE_ROUTING), 
                                                                          mpeSettings,
                                                                          valueTreeState,
                                                                          "Pressure", 
                                                                          id::PRESSURE);  
        timbreChannelComponent = std::make_unique<MPEChannelComponent> (settings.getChildWithName (id::MPE_ROUTING), 
                                                                        mpeSettings,
                                                                        valueTreeState,
                                                                        "Timbre", 
                                                                        id::TIMBRE);
        addAndMakeVisible (pressureChannelComponent.get());
        addAndMakeVisible (timbreChannelComponent.get());
        resized(); repaint();
        std::cout << getTopLevelComponent()->getName() << std::endl;
    }
};
}