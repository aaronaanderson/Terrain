#pragma once

#include "Panel.h"
#include "AttachedInterfaces.h"

#include "../DSP/MPEVoiceData.h"
namespace ti
{
class OutputLevel : public juce::Component 
{
public:
    OutputLevel (juce::AudioProcessorValueTreeState& vts,
                 tp::MPEVoiceData& vd)
      : level ("Output Level", "OutputLevel", vts, vd)
    {
        addAndMakeVisible (level);
    }
    void paint (juce::Graphics& g) override 
    {
        g.setColour (juce::Colours::black);
        g.drawRect (getLocalBounds());
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        level.setBounds (b.removeFromLeft (b.getWidth()));
    }
private:
    KnobParameterSlider level;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputLevel)
};
class Compressor : public juce::Component 
{
public:
    Compressor (juce::AudioProcessorValueTreeState& vts,
                tp::MPEVoiceData& vd)
      : threshold ("Threshold", "CompressorThreshold", vts, vd), 
        ratio ("Ratio", "CompressorRatio", vts, vd)
    {
        label.setText ("Compressor", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        addAndMakeVisible (threshold);
        addAndMakeVisible (ratio);
    }
    void paint (juce::Graphics& g) override 
    {
        g.setColour (juce::Colours::black);
        g.drawRect (getLocalBounds());
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromTop (20));
        auto unitWidth = b.getWidth() / 2.0f;
        threshold.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
        ratio.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
    }
private:
    morph::AutoFitTextBox label;
    KnobParameterSlider threshold, ratio;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Compressor)
};
class PerVoiceFilter : public juce::Component
{
public:
    PerVoiceFilter (juce::AudioProcessorValueTreeState& vts, 
                    tp::MPEVoiceData& vd)
      : perVoiceFrequency ("Frequency", "Per-VoiceFilterFrequency", vts, vd),
        perVoiceResonance ("Resonance", "Per-VoiceFilterResonance", vts, vd),
        perVoiceOnOff ("", "Per-VoiceFilterOnOff", vts)
    {
        label.setText ("Per-Voice Filter", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);

        addAndMakeVisible (perVoiceFrequency);
        addAndMakeVisible (perVoiceResonance);
        addAndMakeVisible (perVoiceOnOff);

    }
    void paint (juce::Graphics& g) override 
    {
        g.setColour (juce::Colours::black);
        g.drawRect (getLocalBounds());
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        int topMargin = juce::roundToInt (b.getHeight() * 0.15f);
        juce::Rectangle<int> onOffBounds {topMargin, topMargin};
        perVoiceOnOff.setBounds (onOffBounds.reduced (juce::roundToInt (b.getHeight() * 0.02f)));

        label.setBounds (b.removeFromTop (topMargin));
        auto unitWidth = b.getWidth() / 2.0f;
        perVoiceFrequency.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
        perVoiceResonance.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
    }
private:
    morph::AutoFitTextBox label;
    KnobParameterSlider perVoiceFrequency, perVoiceResonance;
    ParameterToggle perVoiceOnOff;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerVoiceFilter)
};
class Filter : public juce::Component 
{
public:
    Filter (juce::AudioProcessorValueTreeState& vts, 
            tp::MPEVoiceData& vd)
      : frequency ("Frequency", "FilterFrequency", vts, vd), 
        resonance ("Resonance", "FilterResonance", vts, vd), 
        onOff ("", "FilterOnOff", vts)
    {
        label.setText ("Global Filter", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        addAndMakeVisible (frequency);
        addAndMakeVisible (resonance);
        addAndMakeVisible (onOff);
    }
    void paint (juce::Graphics& g) override 
    {
        g.setColour (juce::Colours::black);
        g.drawRect (getLocalBounds());
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        int topMargin = juce::roundToInt (b.getHeight() * 0.15f);
        juce::Rectangle<int> onOffBounds {topMargin, topMargin};
        onOff.setBounds (onOffBounds.reduced (juce::roundToInt (b.getHeight() * 0.02f)));
        label.setBounds (b.removeFromTop (topMargin));
        auto unitWidth = b.getWidth() / 2.0f;
        frequency.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
        resonance.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
    }
private:
    morph::AutoFitTextBox label;
    KnobParameterSlider frequency, resonance;
    ParameterToggle onOff;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Filter)
};
class Envelope : public juce::Component
{
public:
    Envelope (juce::AudioProcessorValueTreeState& vts, 
              tp::MPEVoiceData& vd)
      : envelopeSize ("ES", "EnvelopeSize", vts),
        attack ("Attack","Attack", vts, vd),
        decay ("Decay","Decay", vts, vd),
        sustain ("Sustain","Sustain", vts, vd),
        release ("Release","Release", vts, vd), 
        sensitivity ("Sensitivity", "Sensitivity", vts, vd)
    {
        label.setText ("Envelope", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        addAndMakeVisible (envelopeSize);
        addAndMakeVisible (attack);
        addAndMakeVisible (decay);
        addAndMakeVisible (sustain);
        addAndMakeVisible (release);
        addAndMakeVisible (sensitivity);
    }
    void paint (juce::Graphics& g) override 
    {
        auto b = getLocalBounds();
        g.setColour (juce::Colours::black);
        g.drawRect (b);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        int topMargin = juce::roundToInt (b.getHeight() * 0.15f);
        label.setBounds (b.removeFromTop (topMargin));
        auto esBounds = b.removeFromLeft (juce::roundToInt (b.getWidth() * 0.1f));
        envelopeSize.setBounds (esBounds.reduced ( juce::roundToInt (esBounds.getWidth() * 0.2f)));

        auto unitWidth = juce::roundToInt (b.getWidth() / 5.0f);
        attack.setBounds (b.removeFromLeft (unitWidth));
        decay.setBounds (b.removeFromLeft (unitWidth));
        sustain.setBounds (b.removeFromLeft ((unitWidth)));
        release.setBounds (b.removeFromLeft (unitWidth));
        sensitivity.setBounds (b.removeFromLeft (unitWidth));
    }

private:
    morph::AutoFitTextBox label;
    ti::ParameterToggle envelopeSize;
    ti::KnobParameterSlider attack, decay, sustain, release, sensitivity;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Envelope)
};
class ControlPanel : public Panel
{
public:
    ControlPanel (juce::AudioProcessorValueTreeState& vts, 
                  tp::MPEVoiceData& vd)
      : Panel ("Control Panel"), 
        envelope (vts, vd),
        perVoiceFilter (vts, vd), 
        filter (vts, vd), 
        compressor (vts, vd), 
        outputLevel (vts, vd)
    {
        addAndMakeVisible (envelope);  
        addAndMakeVisible (perVoiceFilter);
        addAndMakeVisible (filter);
        addAndMakeVisible (compressor);
        addAndMakeVisible (outputLevel);
    }
    void resized() override 
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        auto unitWidth = b.getWidth() / 11.0f;
        envelope.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 4.0f)));
        perVoiceFilter.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 2.0f)));
        filter.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 2.0f)));
        compressor.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 2.0f)));
        outputLevel.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
    }
private:
    Envelope envelope;
    PerVoiceFilter perVoiceFilter;
    Filter filter;
    Compressor compressor;
    OutputLevel outputLevel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPanel)
};
}