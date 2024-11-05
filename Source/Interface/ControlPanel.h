#pragma once

#include "Panel.h"
#include "AttachedInterfaces.h"
namespace ti
{
class OutputLevel : public juce::Component 
{
public:
    OutputLevel (juce::AudioProcessorValueTreeState& vts)
      : level ("Output Level", "OutputLevel", vts)
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
    ParameterSlider level;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputLevel)
};
class Compressor : public juce::Component 
{
public:
    Compressor (juce::AudioProcessorValueTreeState& vts)
      : threshold ("Threshold", "CompressorThreshold", vts), 
        ratio ("Ratio", "CompressorRatio", vts)
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
    juce::Label label;
    ParameterSlider threshold, ratio;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Compressor)
};
class Filter : public juce::Component 
{
public:
    Filter (juce::AudioProcessorValueTreeState& vts)
      : frequency ("Frequency", "FilterFrequency", vts), 
        resonance ("Resonance", "FilterResonance", vts), 
        onOff ("", "FilterOnOff", vts)
    {
        label.setText ("Filter", juce::dontSendNotification);
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
        onOff.setBounds (0, 0, 22, 22);
        label.setBounds (b.removeFromTop (20));
        auto unitWidth = b.getWidth() / 2.0f;
        frequency.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
        resonance.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
    }
private:
    juce::Label label;
    ParameterSlider frequency, resonance;
    ParameterToggle onOff;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Filter)
};
class Envelope : public juce::Component
{
public:
    Envelope (juce::AudioProcessorValueTreeState& vts)
      : envelopeSize ("ES", "EnvelopeSize", vts),
        attack ("Attack","Attack", vts),
        decay ("Decay","Decay", vts),
        sustain ("Sustain","Sustain", vts),
        release ("Release","Release", vts)
    {
        label.setText ("Envelope", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        addAndMakeVisible (envelopeSize);
        addAndMakeVisible (attack);
        addAndMakeVisible (decay);
        addAndMakeVisible (sustain);
        addAndMakeVisible (release);
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
        label.setBounds (b.removeFromTop (20));
        auto unitWidth = b.getWidth() / 43.0f;
        envelopeSize.setBounds (b.removeFromLeft (static_cast<int> (juce::jmax (unitWidth * 3.0f, 22.0f))));
        attack.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 10.0f)));
        decay.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 10.0f)));
        sustain.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 10.0f)));
        release.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 10.0f)));
    }

private:
    juce::Label label;
    ti::ParameterToggle envelopeSize;
    ti::ParameterSlider attack, decay, sustain, release;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Envelope)
};
class ControlPanel : public Panel
{
public:
    ControlPanel (juce::AudioProcessorValueTreeState& vts)
      : Panel ("Control Panel"), 
        envelope (vts), 
        filter (vts), 
        compressor (vts), 
        outputLevel (vts)
    {
        addAndMakeVisible (envelope);  
        addAndMakeVisible (filter);
        addAndMakeVisible (compressor);
        addAndMakeVisible (outputLevel);
    }
    void resized() override 
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        auto unitWidth = b.getWidth() / 10.0f;
        envelope.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 4.0f)));
        filter.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 2.0f)));
        compressor.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 2.0f)));
        outputLevel.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
    }
private:
    Envelope envelope;
    Filter filter;
    Compressor compressor;
    OutputLevel outputLevel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPanel)
};
}