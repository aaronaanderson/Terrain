#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "GlobalTimer.h"
#include "../Parameters.h"

namespace ti
{
struct ParameterToggle : public juce::Component,
                         private GlobalTimer::Listener,
                         private juce::AudioProcessorParameter::Listener
{
    ParameterToggle (juce::AudioProcessorParameter* p,
                     GlobalTimer& gt, 
                     juce::String labelText)
      : parameter (p)
    {
        label.setText (labelText, juce::dontSendNotification);
        //label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        addAndMakeVisible (toggle);
        parameter->addListener (this);
        gt.addListener (*this);
    }
    ~ParameterToggle() override
    {
        parameter->removeListener (this);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        if (label.getText() != "")
            label.setBounds (b.removeFromTop (20));
        toggle.setBounds (b);
    }

    juce::ToggleButton& getToggle() { return toggle; }
private:
    juce::AudioProcessorParameter* parameter;
    juce::ToggleButton toggle;
    juce::Label label;
    bool needsRepainted = true;
    void onTimerCallback() override 
    { 
        if (needsRepainted)
        {
            toggle.setToggleState (static_cast<bool> (parameter->getValue()), juce::dontSendNotification);
            repaint(); 
            needsRepainted = false;
        }
    }
    void parameterValueChanged (int parameterIndex, float newValue) override 
    {
        juce::ignoreUnused (parameterIndex, newValue);
        needsRepainted = true;
    }
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override { juce::ignoreUnused (parameterIndex, gestureIsStarting); }
};
struct ParameterSlider : public juce::Component,
                         private GlobalTimer::Listener,
                         private juce::AudioProcessorParameter::Listener
{
    ParameterSlider (juce::AudioProcessorParameter* p,
                     GlobalTimer& gt, 
                     juce::String labelText, 
                     juce::Range<double> range, 
                     std::optional<double> midPoint = std::optional<double>())
      : parameter (p)
    {
        label.setText (labelText, juce::dontSendNotification);
        
        slider.setRange (range, 0.0);
        if (midPoint.has_value()) slider.setSkewFactorFromMidPoint (midPoint.value());
        slider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 20, 20);

        addAndMakeVisible (label);
        addAndMakeVisible (slider);
        parameter->addListener (this);
        gt.addListener (*this);
    }
    ~ParameterSlider() override
    {
        parameter->removeListener (this);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        if (label.getText().length() > 1)
            label.setBounds (b.removeFromTop (20));
        else
            label.setBounds (b.removeFromLeft (20));
        
        if (b.getHeight() * 2 >= b.getWidth())
        {
            slider.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
            
            label.setJustificationType (juce::Justification::centred);
        }

        slider.setBounds (b);
    }

    void setValue (double v) { slider.setValue (v, juce::NotificationType::sendNotification); }
    juce::Slider& getSlider() { return slider; }
private:
    juce::AudioProcessorParameter* parameter;
    juce::Slider slider;

    juce::Label label;
    bool needsRepainted = true;
    void onTimerCallback() override 
    { 
        if (needsRepainted)
        {
            auto rangedParam = dynamic_cast<juce::RangedAudioParameter*> (parameter);
            jassert (rangedParam != nullptr);
            auto rangedValue = rangedParam->convertFrom0to1 (parameter->getValue());
            slider.setValue (static_cast<double> (rangedValue), juce::dontSendNotification);
            repaint(); 
            needsRepainted = false;
        }
    }
    void parameterValueChanged (int parameterIndex, float newValue) override 
    {
        juce::ignoreUnused (parameterIndex, newValue);
        needsRepainted = true;
    }
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override { juce::ignoreUnused (parameterIndex, gestureIsStarting); }
};
}