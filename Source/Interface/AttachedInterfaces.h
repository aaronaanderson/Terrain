#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include <morphlib/AutoFitTextBox.h>

#include "LookAndFeel.h"
#include "SettingsComponent.h"
#include "VoicesMeter.h"

#include "../DSP/MPEVoiceData.h"

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
        label.setJustificationType (juce::Justification::centred);
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
    morph::AutoFitTextBox label;
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
//struct ParameterSlider : public juce::Component,
//                         public juce::DragAndDropTarget, 
//                         private juce::ValueTree::Listener
//{
//    ParameterSlider (juce::String labelText, 
//                     const juce::String pID, 
//                     juce::AudioProcessorValueTreeState& vts,
//                     tp::MPEVoiceData& voiceData)
//      : voicesMeter (voiceData, vts.state.getChildWithName (id::PRESET_SETTINGS).getChildWithName (id::MPE_ROUTING)),
//        paramID (pID), 
//        valueTreeState (vts)
//    {
//        valueTreeState.state.addListener (this);
//        checkIfControlled();
//        
//        label.setText (labelText, juce::dontSendNotification);
//        slider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 20, 20);
//
//        addAndMakeVisible (label);
//        addAndMakeVisible (slider);
//        addChildComponent (voicesMeter);
//        sliderAttachment.reset (new SliderAttachment (vts, paramID, slider));
//
//        ownershipChanged();
//    }
//    ~ParameterSlider() override { valueTreeState.state.removeListener (this); }
//    void paint (juce::Graphics& g) override
//    {
//        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
//        if (itemDragHovering || isControlled)
//        {
//            g.setColour (laf->getBackgroundDark());
//            g.drawRect (getLocalBounds().toFloat(), 4.0f);
//        }
//        if (isControlled)
//        {
//            g.setColour (laf->getBackgroundColour().darker());
//            g.fillRect (getLocalBounds().toFloat());
//            g.setColour (laf->getBackgroundDark());
//            g.drawRect (getLocalBounds().toFloat(), 4.0f);
//        }
//    }
//    void resized() override 
//    {
//        auto b = getLocalBounds();
//        
//        if (label.getText().length() > 1)
//            label.setBounds (b.removeFromTop (static_cast<int> (b.getHeight() / 3.0f)));
//        else
//            label.setBounds (b.removeFromLeft (static_cast<int> (b.getWidth() / 12.0f)));
//        
//        
//        if (b.getHeight() > b.getWidth() * 1.5)
//        {
//            slider.setSliderStyle (juce::Slider::SliderStyle::LinearVertical);
//            label.setJustificationType (juce::Justification::centred);
//        }
//        else if (b.getHeight() * 2 >= b.getWidth())
//        {
//            slider.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
//            label.setJustificationType (juce::Justification::centred);
//        } 
//        else
//        {
//            slider.setSliderStyle (juce::Slider::SliderStyle::LinearHorizontal);
//            label.setJustificationType (juce::Justification::left);
//        }
//        slider.setBounds (b);
//        voicesMeter.setBounds (b);
//    }
//    // DragAndDropTarget ================================================================
//    bool isInterestedInDragSource (const juce::DragAndDropTarget::SourceDetails& sd) override
//    {
//        juce::ignoreUnused (sd);
//        
//        static const juce::StringArray blockedIDs {
//            "outputLevel", "filterFreq", "filterRes", "compThresh", "compRatio"
//        };
//        
//        if (blockedIDs.contains (paramID))
//            return false;
//        
//        if (isControlled)
//            return false;
//        
//        return true;
//    }
//    void itemDragEnter (const juce::DragAndDropTarget::SourceDetails& sd) override 
//    {
//        juce::ignoreUnused (sd);
//        itemDragHovering = true; repaint();
//    }
//    void itemDragExit (const juce::DragAndDropTarget::SourceDetails& sd) override
//    {
//        juce::ignoreUnused (sd);
//        itemDragHovering = false; repaint();
//    }
//    void itemDropped (const juce::DragAndDropTarget::SourceDetails& sd) override
//    {
//        juce::ignoreUnused (sd);
//        itemDragHovering = false; repaint();
//        
//        auto* draggableSource = dynamic_cast<DraggableAssigner*> (sd.sourceComponent.get());
//        juce::ValueTree channelRouting = draggableSource->getMPEChannelRouting();
//        auto name = valueTreeState.getParameter (paramID)->getName (40);
//        draggableSource->setLabel (name);
//        channelRouting.setProperty (id::name, paramID, nullptr);
//
//        valueTreeState.state.getChildWithName (id::MPE_ROUTING);
//        checkIfControlled();
//    }
//private:
//    juce::Slider slider;
//    VoiceMeter voicesMeter;
//    std::unique_ptr<SliderAttachment> sliderAttachment;
//    juce::Label label;
//    const juce::String paramID;
//    juce::AudioProcessorValueTreeState& valueTreeState;
//
//    bool itemDragHovering = false;
//    bool isControlled = false;
//
//    void valueTreeRedirected (juce::ValueTree& tree) override
//    {
//        juce::ignoreUnused (tree);
//        checkIfControlled();
//        voicesMeter.setRoutingState (valueTreeState.state.getChildWithName (id::MPE_ROUTING));
//    }
//    void valueTreePropertyChanged (juce::ValueTree& tree,
//                                  const juce::Identifier& property) override
//    {
//        juce::ignoreUnused (tree);
//        if (property == id::name) checkIfControlled();
//
//        if (property == id::mpeEnabled)
//        {
//            if (!tree.getProperty (property))
//            {
//                isControlled = false;
//                ownershipChanged();
//                repaint();
//            }else { checkIfControlled(); }    
//        }
//    }
//    void checkIfControlled()
//    {
//        juce::Array<juce::Identifier> ids {id::OUTPUT_ONE, id::OUTPUT_TWO, id::OUTPUT_THREE,
//                                           id::OUTPUT_FOUR, id::OUTPUT_FIVE, id::OUTPUT_SIX};
//        auto routingBranch = valueTreeState.state.getChildWithName (id::PRESET_SETTINGS)
//                                                 .getChildWithName (id::MPE_ROUTING);
//        auto pressureBranch = routingBranch.getChildWithName (id::PRESSURE);
//        isControlled = false;
//        for (auto id : ids)
//        {
//            if (pressureBranch.getChildWithName (id).getProperty (id::name).toString() == paramID)
//            {
//                voicesMeter.setOutputID (id);
//                voicesMeter.setMPEChannel (id::PRESSURE);
//                isControlled = true;
//            } 
//        }
//        auto timbreBranch = routingBranch.getChildWithName (id::TIMBRE);
//        for (auto id : ids)
//        {
//            if (timbreBranch.getChildWithName (id).getProperty (id::name).toString() == paramID)
//            {
//                voicesMeter.setOutputID (id);
//                voicesMeter.setMPEChannel (id::TIMBRE);
//                isControlled = true;
//            } 
//        }
//        ownershipChanged();
//        repaint();
//    }
//    void ownershipChanged()
//    {
//        if (isControlled)
//        {
//            slider.setVisible (false);
//            voicesMeter.setVisible (true);
//        }
//        else
//        {
//            slider.setVisible (true);
//            voicesMeter.setVisible (false);
//        }
//    }
//
//    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterSlider)
//};















// Forward decls you already have:
// struct VoiceMeter;
// namespace tp { struct MPEVoiceData; }
// namespace id { extern const juce::Identifier PRESET_SETTINGS, MPE_ROUTING, PRESSURE, TIMBRE,
//                OUTPUT_ONE, OUTPUT_TWO, OUTPUT_THREE, OUTPUT_FOUR, OUTPUT_FIVE, OUTPUT_SIX,
//                name, mpeEnabled; }
// struct DraggableAssigner;

struct ParameterSliderBase : public juce::Component,
                             public juce::DragAndDropTarget,
                             private juce::ValueTree::Listener
{
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    ParameterSliderBase (const juce::String& labelTextIn,
                         const juce::String& paramIDIn,
                         juce::AudioProcessorValueTreeState& apvts,
                         tp::MPEVoiceData& voiceData)
        : valueTreeState (apvts),
          paramID (paramIDIn),
          labelText (labelTextIn),
          voicesMeter (voiceData,
                       apvts.state.getChildWithName (id::PRESET_SETTINGS)
                                   .getChildWithName (id::MPE_ROUTING))
    {
        label.setText     (labelText, juce::dontSendNotification);
        addAndMakeVisible (label);
        addChildComponent (slider);
        addChildComponent (voicesMeter);
    
        bindToParam (paramID);
    
        valueTreeState.state.addListener (this);
    
        updateControlledState();
    }

    ~ParameterSliderBase() override { valueTreeState.state.removeListener (this); }

    // Subclass hook: layout & slider style
    virtual void applyLayout (juce::Rectangle<int> bounds) = 0;

    void paint (juce::Graphics& g) override
    {
        auto* tlaf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        jassert (tlaf != nullptr);

        const auto r = getLocalBounds().toFloat();

        if (itemDragHovering || isControlled)
        {
            g.setColour (tlaf->getBackgroundDark());
            g.drawRect (r, 4.0f);
        }
        if (isControlled)
        {
            g.setColour (tlaf->getBackgroundColour().darker());
            g.fillRect (r);
            g.setColour (tlaf->getBackgroundDark());
            g.drawRect (r, 4.0f);
        }
    }

    void resized() override
    {
        applyLayout (getLocalBounds());
        // Meter overlays control region
        voicesMeter.setBounds (slider.getBounds());
    }

    // ---------- Drag & Drop ----------
    bool isInterestedInDragSource (const SourceDetails& sd) override
    {
        juce::ignoreUnused (sd);
        if (isControlled) return false;
        return ! isBlockedParamID (paramID);
    }

    void itemDragEnter (const SourceDetails& sd) override
    {
        juce::ignoreUnused (sd);
        itemDragHovering = true; repaint();
    }
    void itemDragExit (const SourceDetails& sd) override
    {
        juce::ignoreUnused (sd);
        itemDragHovering = false; repaint();
    }
    void itemDropped (const SourceDetails& sd) override
    {
        itemDragHovering = false;
        repaint();
    
        if (auto* src = dynamic_cast<DraggableAssigner*> (sd.sourceComponent.get()))
        {
            auto channelRouting = src->getMPEChannelRouting();
    
            if (auto* p = valueTreeState.getParameter (paramID))
                src->setLabel (p->getName (40));
    
            if (channelRouting.isValid())
                channelRouting.setProperty (id::name, paramID, nullptr);
    
            updateControlledState();
            
        }
    }


protected:
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::RangedAudioParameter* param = nullptr;

    morph::AutoFitTextBox  label;
    juce::Slider           slider;
    VoiceMeter             voicesMeter;

    std::unique_ptr<SliderAttachment> sliderAttachment;

    juce::String labelText;
    juce::String paramID;

    bool itemDragHovering = false;
    bool isControlled     = false;

    static bool isBlockedParamID (const juce::String& idStr)
    {
        // Keep this in IDs, not display names
        static const juce::String blocked[] {
            "outputLevel", "filterFreq", "filterQ", "compThreshold", "compRatio"
        };
        for (const auto& s : blocked)
            if (idStr == s) return true;
        return false;
    }

    void bindToParam (const juce::String& idStr)
    {
        param = valueTreeState.getParameter (idStr);
        sliderAttachment.reset();

        if (param != nullptr)
        {
            slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
            sliderAttachment = std::make_unique<SliderAttachment> (valueTreeState, idStr, slider);
        }
        else
        {
            jassertfalse; // invalid parameter ID
        }
    }

    juce::ValueTree routingTree() const
    {
        auto preset = valueTreeState.state.getChildWithName (id::PRESET_SETTINGS);
        return preset.getChildWithName (id::MPE_ROUTING);
    }

    static juce::ValueTree child (const juce::ValueTree& vt, const juce::Identifier& name)
    {
        return vt.getChildWithName (name);
    }

    static bool isParamRoutedToBranch (const juce::ValueTree& branch,
                                       const juce::String& targetParamID,
                                       juce::Identifier& matchedOutputID)
    {
        static const juce::Identifier outputs[] {
            id::OUTPUT_ONE, id::OUTPUT_TWO, id::OUTPUT_THREE,
            id::OUTPUT_FOUR, id::OUTPUT_FIVE, id::OUTPUT_SIX
        };

        for (auto outId : outputs)
        {
            const auto node = child (branch, outId);
            if (node.isValid() && node.getProperty (id::name).toString() == targetParamID)
            {
                matchedOutputID = outId;
                return true;
            }
        }
        return false;
    }

    void setControlled (bool controlled)
    {
        isControlled = controlled;
        slider.setVisible (!controlled);
        voicesMeter.setVisible (controlled);
        repaint();
    }

    void updateControlledState()
    {
        const auto routing   = routingTree();

        bool controlled = false;
        juce::Identifier outId;

        auto pressure = child (routing, id::PRESSURE);
        auto timbre   = child (routing, id::TIMBRE);

        if (isParamRoutedToBranch (pressure, paramID, outId))
        {
            voicesMeter.setOutputID (outId);
            voicesMeter.setMPEChannel (id::PRESSURE);
            controlled = true;
        }
        else if (isParamRoutedToBranch (timbre, paramID, outId))
        {
            voicesMeter.setOutputID (outId);
            voicesMeter.setMPEChannel (id::TIMBRE);
            controlled = true;
        }

        setControlled (controlled);
    }

private:
    // ValueTree::Listener
    void valueTreeRedirected (juce::ValueTree& t) override
    {
        juce::ignoreUnused (t);
        voicesMeter.setRoutingState (routingTree());
        updateControlledState();
    }

    void valueTreePropertyChanged (juce::ValueTree& tree,
                                   const juce::Identifier& property) override
    {
        juce::ignoreUnused (tree);
        if (property == id::name || property == id::mpeEnabled)
            updateControlledState();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterSliderBase)
};

//==============================================================
// LinearParameterSlider: Layout::Wide (horizontal)
//==============================================================
struct LinearParameterSlider : public ParameterSliderBase
{
    LinearParameterSlider (const juce::String& labelTextIn,
                           const juce::String& paramIDIn,
                           juce::AudioProcessorValueTreeState& apvts,
                           tp::MPEVoiceData& voiceData)
        : ParameterSliderBase (labelTextIn, paramIDIn, apvts, voiceData)
    {
        slider.setSliderStyle (juce::Slider::LinearHorizontal);
        label.setJustificationType (juce::Justification::left);
    }

    void applyLayout (juce::Rectangle<int> bounds) override
    {
        auto b = bounds;
        const bool hasTitle = label.getText().isNotEmpty();
        if (hasTitle)
            label.setBounds (b.removeFromLeft (juce::jmax (40, b.getWidth() / 6)));
        else
            label.setBounds ({});

        slider.setBounds (b);
    }
};

//==============================================================
// KnobParameterSlider: Layout::Tall (rotary knob)
//==============================================================
struct KnobParameterSlider : public ParameterSliderBase
{
    KnobParameterSlider (const juce::String& labelTextIn,
                         const juce::String& paramIDIn,
                         juce::AudioProcessorValueTreeState& apvts,
                         tp::MPEVoiceData& voiceData)
        : ParameterSliderBase (labelTextIn, paramIDIn, apvts, voiceData)
    {
        slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        label.setJustificationType (juce::Justification::centred);
    }

    void applyLayout (juce::Rectangle<int> bounds) override
    {
        auto b = bounds;
        const bool hasTitle = label.getText().isNotEmpty();
        if (hasTitle)
            label.setBounds (b.removeFromTop (b.getHeight() / 4));
        else
            label.setBounds ({});

        slider.setBounds (b);
    }
};


}