#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "GlobalTimer.h"
#include "../Parameters.h"

namespace ti
{
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
        
        slider.setBounds (b);
    }

    void setValue (double v) { slider.setValue (v, juce::NotificationType::dontSendNotification); }
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