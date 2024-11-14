#pragma once

#include "Panel.h"
#include "AttachedInterfaces.h"
#include "../Parameters.h"
namespace ti
{
class ModifierArray : public juce::Component,
                      private juce::ValueTree::Listener
{
public:
    ModifierArray (juce::AudioProcessorValueTreeState& vts, 
                   juce::ValueTree voicesState)
      : aModifier ("a", "TrajectoryModA", vts, voicesState),
        bModifier ("b", "TrajectoryModB", vts, voicesState),
        cModifier ("c", "TrajectoryModC", vts, voicesState),
        dModifier ("d", "TrajectoryModD", vts, voicesState)
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
    void setVisibleSliders (int numberVisible)
    {
        jassert (numberVisible <= 4);
        aModifier.setVisible (false);
        bModifier.setVisible (false);
        cModifier.setVisible (false);
        dModifier.setVisible (false);
        switch (numberVisible)
        {
            case 0: break;
            case 1:
                aModifier.setVisible (true);
            break;
            case 2:
                aModifier.setVisible (true);
                bModifier.setVisible (true);
            break;
            case 3:
                aModifier.setVisible (true);
                bModifier.setVisible (true);
                cModifier.setVisible (true);
            break;
            case 4:
                aModifier.setVisible (true);
                bModifier.setVisible (true);
                cModifier.setVisible (true);
                dModifier.setVisible (true);
            break;
        }
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
    TrajectorySelector (juce::AudioProcessorValueTreeState& vts, 
                        juce::ValueTree voicesState)
      : modifierArray (vts, voicesState),
        trajectoryList ("CurrentTrajectory", vts, resetModifierArray)
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
        trajectoryList.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 2.0f)).withX (2)
                                                                                        .withWidth (b.getWidth() - 4));
        modifierArray.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 8.0f)));
    }
    std::function<void()> resetModifierArray = [&]()
        {
            auto numberOfVisibleSliders = trajectoryNameToVisibleSliders (trajectoryList.getCurrent());
            modifierArray.setVisibleSliders (numberOfVisibleSliders);
        };
private:
    ModifierArray modifierArray;
    ParameterComboBox trajectoryList;
    juce::Label trajectoryListLabel;

    int trajectoryNameToVisibleSliders (juce::String trajectoryName)
    {
        if (trajectoryName == "Ellipse") return 1;
        else if (trajectoryName == "Superellipse") return 3;
        else if (trajectoryName == "Limacon") return 2;
        else if (trajectoryName == "Butterfly") return 1;
        else if (trajectoryName == "Scarabaeus") return 2;
        else if (trajectoryName == "Squarcle") return 1;
        else if (trajectoryName == "Bicorn") return 0;
        else if (trajectoryName == "Cornoid") return 1;
        else if (trajectoryName == "Epitrochoid 3") return 1;
        else if (trajectoryName == "Epitrochoid 5") return 1;
        else if (trajectoryName == "Epitrochoid 7") return 1;
        else if (trajectoryName == "Hypocycloid 3") return 1;
        else if (trajectoryName == "Hypocycloid 5") return 1;
        else if (trajectoryName == "Hypocycloid 7") return 1;
        else if (trajectoryName == "Gear Curve 3") return 1;
        else if (trajectoryName == "Gear Curve 5") return 1;
        else if (trajectoryName == "Gear Curve 7") return 1;
        jassertfalse; 
        return 0;
    }
   
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectorySelector)      
};
class FeedbackPanel : public juce::Component
{
public:
    FeedbackPanel (juce::AudioProcessorValueTreeState& vts, 
                   juce::ValueTree voicesState)
      : time ("Time", "FeedbackTime", vts, voicesState), 
        feedback ("Feedback", "Feedback", vts, voicesState), 
        mix ("Mix", "FeedbackMix", vts, voicesState),
        compression ("Compression", "FeedbackCompression", vts, voicesState)
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
    TrajectoryVariables (juce::AudioProcessorValueTreeState& vts, 
                         juce::ValueTree voicesState)
      : amplitude ("Amplitude", "Amplitude", vts, voicesState),
        size ("Size", "Size", vts, voicesState),
        rotation ("Rotation", "Rotation", vts, voicesState),
        translation_x ("Translation X", "TranslationX", vts, voicesState),
        translation_y ("Translation Y", "TranslationY", vts, voicesState)
    {
        addAndMakeVisible (amplitude);
        addAndMakeVisible (size);
        addAndMakeVisible (rotation);
        addAndMakeVisible (translation_x);
        addAndMakeVisible (translation_y);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        auto unitHeight = b.getHeight() / static_cast<float> (5);
        amplitude.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        size.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        rotation.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        translation_x.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        translation_y.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
    }
private:
    ParameterSlider amplitude;
    ParameterSlider size;
    ParameterSlider rotation;
    ParameterSlider translation_x;
    ParameterSlider translation_y;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryVariables)
};
class MeanderancePanel : public juce::Component
{
public:
    MeanderancePanel (juce::AudioProcessorValueTreeState& vts,
                      juce::ValueTree voicesState)
      : scale ("Scale", "MeanderanceScale", vts, voicesState),
        speed ("Speed", "MeanderanceSpeed", vts, voicesState)
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
    TrajectoryPanel (juce::AudioProcessorValueTreeState& vts, 
                     juce::ValueTree voicesState)
      : Panel ("Trajectory"),  
        trajectorySelector (vts, voicesState),
        trajectoryVariables (vts, voicesState),
        meanderancePanel (vts, voicesState),
        feedbackPanel (vts, voicesState)
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
        auto unitHeight = b.getHeight() / static_cast<float> ((12 + 20 + 10 + 22));
        trajectorySelector.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 12.0f)));
        trajectoryVariables.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 20.0f)));
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