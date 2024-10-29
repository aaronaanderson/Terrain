#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>

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
    ParameterComboBox (const juce::String parmID, juce::AudioProcessorValueTreeState& vts)
    {
        comboBoxAttachment.reset (new ComboBoxAttachment (vts, parmID, comboBox));
    }
    void resized() override { comboBox.setBounds (getLocalBounds()); }
private:
    juce::ComboBox comboBox;
    std::unique_ptr<ComboBoxAttachment> comboBoxAttachment;
};

struct ParameterSlider : public juce::Component
{
    ParameterSlider (juce::String labelText, 
                     const juce::String paramID, 
                     juce::AudioProcessorValueTreeState& vts)

    {
        label.setText (labelText, juce::dontSendNotification);
        slider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 20, 20);

        addAndMakeVisible (label);
        addAndMakeVisible (slider);
        sliderAttachment.reset (new SliderAttachment (vts, paramID, slider));
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        
        if (label.getText().length() > 1)
            label.setBounds (b.removeFromTop (static_cast<int> (b.getHeight() / 3.0f)));
        else
            label.setBounds (b.removeFromLeft (static_cast<int> (b.getWidth() / 12.0f)));
        
        if (b.getHeight() * 2 >= b.getWidth())
        {
            slider.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
            label.setJustificationType (juce::Justification::centred);
        }

        slider.setBounds (b);
    }
private:
    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<SliderAttachment> sliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterSlider)
};
}