#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include "LookAndFeel.h"
#include "SettingsComponent.h"
#include "VoicesMeter.h"

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
namespace ti
{
struct ParameterToggle : public juce::Component
{
    ParameterToggle (juce::String labelText, 
                     const juce::String paramID, 
                     juce::AudioProcessorValueTreeState& vts)
    {
        label.setText (labelText, juce::dontSendNotification);
        addAndMakeVisible (label);
        addAndMakeVisible (toggle);

        buttonAttachment.reset (new ButtonAttachment (vts, paramID, toggle));
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        auto unitHeight = b.getHeight() / 3.0f;
        if (label.getText() != "")
            label.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        toggle.setBounds (b);
    }
private:
    juce::ToggleButton toggle;
    juce::Label label;
    std::unique_ptr<ButtonAttachment> buttonAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterToggle)
};
struct ParameterComboBox : public juce::Component
{
    ParameterComboBox (const juce::String paramID, 
                       juce::AudioProcessorValueTreeState& vts, 
                       std::function<void()> comboBoxChanged = nullptr)
      : onComboBoxChange (comboBoxChanged)
    {
        auto apc = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramID));
        jassert (apc != nullptr);
        auto options = apc->getAllValueStrings();
        comboBox.addItemList (options, 1);
        comboBox.onChange = [&](){ if (onComboBoxChange != nullptr) onComboBoxChange(); };
        comboBoxAttachment.reset (new ComboBoxAttachment (vts, paramID, comboBox));
        addAndMakeVisible (comboBox);
    }
    void resized() override { comboBox.setBounds (getLocalBounds()); }
    void setOptions (juce::StringArray options)
    {
        comboBox.clear();
        comboBox.addItemList (options, 1);
    }
    std::function<void()> onComboBoxChange = nullptr;
    juce::String getCurrent() { return comboBox.getText(); }
private:
    juce::ComboBox comboBox;
    std::unique_ptr<ComboBoxAttachment> comboBoxAttachment;
};
struct ParameterSlider : public juce::Component,
                         public juce::DragAndDropTarget, 
                         private juce::ValueTree::Listener
{
    ParameterSlider (juce::String labelText, 
                     const juce::String pID, 
                     juce::AudioProcessorValueTreeState& vts,
                     juce::ValueTree voicesState = juce::ValueTree())
      : voicesMeter (voicesState, vts.state.getChildWithName (id::PRESET_SETTINGS).getChildWithName (id::MPE_ROUTING)),
        paramID (pID), 
        valueTreeState (vts)
    {
        valueTreeState.state.addListener (this);
        checkIfControlled();
        
        label.setText (labelText, juce::dontSendNotification);
        slider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 20, 20);

        addAndMakeVisible (label);
        addAndMakeVisible (slider);
        addChildComponent (voicesMeter);
        sliderAttachment.reset (new SliderAttachment (vts, paramID, slider));

        ownershipChanged();
    }
    ~ParameterSlider() override { valueTreeState.state.removeListener (this); }
    void paint (juce::Graphics& g) override
    {
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        if (itemDragHovering || isControlled)
        {
            g.setColour (laf->getBackgroundDark());
            g.drawRect (getLocalBounds().toFloat(), 4.0f);
        }
        if (isControlled)
        {
            g.setColour (laf->getBackgroundColour().darker());
            g.fillRect (getLocalBounds().toFloat());
            g.setColour (laf->getBackgroundDark());
            g.drawRect (getLocalBounds().toFloat(), 4.0f);
        }
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        
        if (label.getText().length() > 1)
            label.setBounds (b.removeFromTop (static_cast<int> (b.getHeight() / 3.0f)));
        else
            label.setBounds (b.removeFromLeft (static_cast<int> (b.getWidth() / 12.0f)));
        
        
        if (b.getHeight() > b.getWidth() * 1.5)
        {
            slider.setSliderStyle (juce::Slider::SliderStyle::LinearVertical);
            label.setJustificationType (juce::Justification::centred);
        }
        else if (b.getHeight() * 2 >= b.getWidth())
        {
            slider.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
            label.setJustificationType (juce::Justification::centred);
        } 
        else
        {
            slider.setSliderStyle (juce::Slider::SliderStyle::LinearHorizontal);
            label.setJustificationType (juce::Justification::left);
        }
        slider.setBounds (b);
        voicesMeter.setBounds (b);
    }
    // DragAndDropTarget ================================================================
    bool isInterestedInDragSource (const juce::DragAndDropTarget::SourceDetails& sd) override
    {
        juce::ignoreUnused (sd);
        if (valueTreeState.getParameter (paramID)->getName (30) == "Output Level"         ||
            valueTreeState.getParameter (paramID)->getName (30) == "Filter Frequency"     ||
            valueTreeState.getParameter (paramID)->getName (30) == "Filter Resonance"     ||
            valueTreeState.getParameter (paramID)->getName (30) == "Compressor Threshold" ||
            valueTreeState.getParameter (paramID)->getName (30) == "Compressor Ratio")
            return false;
        if (isControlled) return false;

        return true;
    }
    void itemDragEnter (const juce::DragAndDropTarget::SourceDetails& sd) override 
    {
        juce::ignoreUnused (sd);
        itemDragHovering = true; repaint();
    }
    void itemDragExit (const juce::DragAndDropTarget::SourceDetails& sd) override
    {
        juce::ignoreUnused (sd);
        itemDragHovering = false; repaint();
    }
    void itemDropped (const juce::DragAndDropTarget::SourceDetails& sd) override
    {
        juce::ignoreUnused (sd);
        itemDragHovering = false; repaint();
        
        auto* draggableSource = dynamic_cast<DraggableAssigner*> (sd.sourceComponent.get());
        juce::ValueTree channelRouting = draggableSource->getMPEChannelRouting();
        auto name = valueTreeState.getParameter (paramID)->getName (40);
        draggableSource->setLabel (name);
        channelRouting.setProperty (id::name, paramID, nullptr);

        valueTreeState.state.getChildWithName (id::MPE_ROUTING);
        checkIfControlled();
    }
private:
    juce::Slider slider;
    VoiceMeter voicesMeter;
    std::unique_ptr<SliderAttachment> sliderAttachment;
    juce::Label label;
    const juce::String paramID;
    juce::AudioProcessorValueTreeState& valueTreeState;

    bool itemDragHovering = false;
    bool isControlled = false;

    void valueTreeRedirected (juce::ValueTree& tree) override
    {
        juce::ignoreUnused (tree);
        checkIfControlled();
        voicesMeter.setRoutingState (valueTreeState.state.getChildWithName (id::MPE_ROUTING));
    }
    void valueTreePropertyChanged (juce::ValueTree& tree,
                                  const juce::Identifier& property) override
    {
        juce::ignoreUnused (tree);
        if (property == id::name) checkIfControlled();

        if (property == id::mpeEnabled)
        {
            if (!tree.getProperty (property))
            {
                isControlled = false;
                ownershipChanged();
                repaint();
            }else { checkIfControlled(); }    
        }
    }
    void checkIfControlled()
    {
        juce::Array<juce::Identifier> ids {id::OUTPUT_ONE, id::OUTPUT_TWO, id::OUTPUT_THREE,
                                           id::OUTPUT_FOUR, id::OUTPUT_FIVE, id::OUTPUT_SIX};
        auto routingBranch = valueTreeState.state.getChildWithName (id::PRESET_SETTINGS)
                                                 .getChildWithName (id::MPE_ROUTING);
        auto pressureBranch = routingBranch.getChildWithName (id::PRESSURE);
        isControlled = false;
        for (auto id : ids)
        {
            if (pressureBranch.getChildWithName (id).getProperty (id::name).toString() == paramID)
            {
                voicesMeter.setOutputID (id);
                voicesMeter.setMPEChannel (id::PRESSURE);
                isControlled = true;
            } 
        }
        auto timbreBranch = routingBranch.getChildWithName (id::TIMBRE);
        for (auto id : ids)
        {
            if (timbreBranch.getChildWithName (id).getProperty (id::name).toString() == paramID)
            {
                voicesMeter.setOutputID (id);
                voicesMeter.setMPEChannel (id::TIMBRE);
                isControlled = true;
            } 
        }
        ownershipChanged();
        repaint();
    }
    void ownershipChanged()
    {
        if (isControlled)
        {
            slider.setVisible (false);
            voicesMeter.setVisible (true);
        }
        else
        {
            slider.setVisible (true);
            voicesMeter.setVisible (false);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterSlider)
};
}