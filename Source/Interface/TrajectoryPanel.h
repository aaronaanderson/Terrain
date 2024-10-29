#pragma once

#include "Panel.h"
#include "AttachedInterfaces.h"

namespace ti
{
class ModifierArray : public juce::Component,
                      private juce::ValueTree::Listener
{
public:
    ModifierArray (juce::AudioProcessorValueTreeState& vts)
      : aModifier ("a", "TrajectoryModA", vts),
        bModifier ("b", "TrajectoryModB", vts),
        cModifier ("c", "TrajectoryModC", vts),
        dModifier ("d", "TrajectoryModD", vts)
    {
        addAndMakeVisible (aModifier);
        addAndMakeVisible (bModifier);
        addAndMakeVisible (cModifier);
        addAndMakeVisible (dModifier);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        auto quarterHeight = b.getHeight() / 4;
        aModifier.setBounds (b.removeFromTop (quarterHeight));
        bModifier.setBounds (b.removeFromTop (quarterHeight));
        cModifier.setBounds (b.removeFromTop (quarterHeight));
        dModifier.setBounds (b.removeFromTop (quarterHeight));
    }
private:
    ParameterSlider aModifier;
    ParameterSlider bModifier;
    ParameterSlider cModifier;
    ParameterSlider dModifier;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModifierArray)
};

class TrajectorySelector : public juce::Component
{
public:
    TrajectorySelector (juce::AudioProcessorValueTreeState& vts)
      : trajectoryList ("CurrentTrajectory", vts), 
        modifierArray (vts)
    {
        trajectoryListLabel.setText ("Current Trajectory", juce::NotificationType::dontSendNotification);
        trajectoryListLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (trajectoryList);
        addAndMakeVisible (trajectoryListLabel);
        addAndMakeVisible (modifierArray);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        auto unitHeight = b.getHeight() / static_cast<float> (2 + 2 + 8);
        trajectoryListLabel.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 2.0f)));
        trajectoryList.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 2.0f)));
        modifierArray.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 8.0f)));
    }
private:
    ParameterComboBox trajectoryList;
    juce::Label trajectoryListLabel;
    ModifierArray modifierArray;
   
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectorySelector)      
};
class FeedbackPanel : public juce::Component
{
public:
    FeedbackPanel (juce::AudioProcessorValueTreeState& vts)
      : time ("Time", "FeedbackTime", vts), 
        feedback ("Feedback", "Feedback", vts), 
        mix ("Mix", "FeedbackMix", vts),
        compression ("Compression", "FeedbackCompression", vts)
    {
        label.setText ("Trajectory Feedback", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        addAndMakeVisible (time);
        addAndMakeVisible (feedback);
        addAndMakeVisible (compression);
        addAndMakeVisible (mix);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        auto unitHeight = b.getHeight() / static_cast<float> (2 + 4 + 4 + 4 + 4);
        label.setBounds (b.removeFromTop(static_cast<int> (unitHeight * 2.0f)));
        time.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 4.0f)));
        feedback.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 4.0f)));
        compression.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 4.0f)));
        mix.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 4.0f)));
    }
private:
    juce::Label label;
    ParameterSlider time, feedback, mix, compression;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FeedbackPanel)
};
class TrajectoryVariables : public juce::Component 
{
public:
    TrajectoryVariables (juce::AudioProcessorValueTreeState& vts)
      : size ("Size", "Size", vts),
        rotation ("Rotation", "Rotation", vts),
        translation_x ("Translation X", "TranslationX", vts),
        translation_y ("Translation Y", "TranslationY", vts)
    {
        addAndMakeVisible (size);
        addAndMakeVisible (rotation);
        addAndMakeVisible (translation_x);
        addAndMakeVisible (translation_y);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        auto unitHeight = b.getHeight() / static_cast<float> (4);
        size.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        rotation.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        translation_x.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        translation_y.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
    }
private:
    ParameterSlider size;
    ParameterSlider rotation;
    ParameterSlider translation_x;
    ParameterSlider translation_y;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryVariables)
};
class MeanderancePanel : public juce::Component
{
public:
    MeanderancePanel (juce::AudioProcessorValueTreeState& vts)
      : scale ("Scale", "MeanderanceScale", vts),
        speed ("Speed", "MeanderanceSpeed", vts)
    {
        label.setText ("Meanderance", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        addAndMakeVisible (scale);
        addAndMakeVisible (speed);
    }
    void resized()
    {
        auto b = getLocalBounds();
        auto unitHeight = b.getHeight() / static_cast<float> (2 + 4 + 4);
        label.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 2.0f)));
        scale.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 4.0f)));
        speed.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 4.0f)));
    }
private:
    juce::Label label;
    ParameterSlider scale, speed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeanderancePanel)
};
class TrajectoryPanel : public Panel
{
public:
    TrajectoryPanel (juce::AudioProcessorValueTreeState& vts)
      : Panel ("Trajectory"),  
        trajectorySelector (vts),
        trajectoryVariables (vts),
        meanderancePanel (vts),
        feedbackPanel (vts)
    {
        addAndMakeVisible (trajectorySelector);
        addAndMakeVisible (trajectoryVariables);
        addAndMakeVisible (meanderancePanel);
        addAndMakeVisible (feedbackPanel);
    }
    void resized () override 
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        auto unitHeight = b.getHeight() / static_cast<float> ((12 + 16 + 10 + 22));
        trajectorySelector.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 12.0f)));
        trajectoryVariables.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 16.0f)));
        meanderancePanel.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 10.0f)));
        feedbackPanel.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 22.0f)));
    }
private:
    TrajectorySelector trajectorySelector;
    TrajectoryVariables trajectoryVariables;
    MeanderancePanel meanderancePanel;
    FeedbackPanel feedbackPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryPanel)
};
}