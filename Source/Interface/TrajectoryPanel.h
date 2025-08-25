#pragma once

#include "Panel.h"
#include "AttachedInterfaces.h"
#include "../Parameters.h"
#include "../DSP/MPEVoiceData.h"
#include "morphlib/AutoFitTextBox.h"
namespace ti
{
class ModifierArray : public juce::Component,
                      private juce::ValueTree::Listener
{
public:
    ModifierArray (juce::AudioProcessorValueTreeState& vts, 
                   tp::MPEVoiceData& vd)
      : aModifier ("a", "TrajectoryModA", vts, vd),
        bModifier ("b", "TrajectoryModB", vts, vd),
        cModifier ("c", "TrajectoryModC", vts, vd),
        dModifier ("d", "TrajectoryModD", vts, vd)
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
    LinearParameterSlider aModifier;
    LinearParameterSlider bModifier;
    LinearParameterSlider cModifier;
    LinearParameterSlider dModifier;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModifierArray)
};
class TrajectorySelector : public juce::Component
{
public:
    TrajectorySelector (juce::AudioProcessorValueTreeState& vts, 
                        tp::MPEVoiceData& vd)
      : modifierArray (vts, vd),
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
    morph::AutoFitTextBox trajectoryListLabel;

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
                   tp::MPEVoiceData& vd)
      : time ("Freq", "CombFrequency", vts, vd), 
        feedback ("Feedback", "Feedback", vts, vd), 
        mix ("Mix", "FeedbackMix", vts, vd)
    {
        label.setText ("Spatial Comb", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        addAndMakeVisible (time);
        addAndMakeVisible (feedback);
        addAndMakeVisible (mix);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        int label_height = juce::roundToInt (b.getHeight() * 0.2f);
        label.setBounds( b.removeFromTop( label_height ) );

        auto unitWidth = b.getWidth() / static_cast<float> (4 + 4 + 4);
        time.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 4.0f)));
        feedback.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 4.0f)));
        mix.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 4.0f)));
    }
private:
    morph::AutoFitTextBox label;
    KnobParameterSlider time, feedback, mix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FeedbackPanel)
};
class RadialCompressorPanel : public juce::Component
{
public:
    RadialCompressorPanel (juce::AudioProcessorValueTreeState& vts, 
                   tp::MPEVoiceData& vd)
      : threshold ("Threshold", "RadialCompressorThreshold", vts, vd), 
        ratio ("Ratio", "RadialCompressorRatio", vts, vd), 
        responsiveness ("Responsiveness", "RadialCompressorResponsiveness", vts, vd)
    {
        label.setText ("Radial Compression", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        addAndMakeVisible (threshold);
        addAndMakeVisible (ratio);
        addAndMakeVisible (responsiveness);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        int label_height = juce::roundToInt (b.getHeight() * 0.2f);
        label.setBounds( b.removeFromTop (label_height));
            
        auto unitWidth = b.getWidth() / static_cast<float> (4 + 4 + 4);
        threshold.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 4.0f)));
        ratio.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 4.0f)));
        responsiveness.setBounds (b.removeFromLeft (static_cast<int> (unitWidth * 4.0f)));
    }
private:
    morph::AutoFitTextBox label;
    KnobParameterSlider threshold, ratio, responsiveness;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RadialCompressorPanel)
};

class BandpassPanel : public juce::Component
{
public:
    BandpassPanel (juce::AudioProcessorValueTreeState& vts, 
                   tp::MPEVoiceData& vd)
      : centerFreq ("Center", "VoiceBandPassCenterFreq", vts, vd), 
        bandwidth ("Bandwidth", "VoiceBandPassBandwidth", vts, vd) 
    {
        label.setText ("Bandpass", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        addAndMakeVisible (centerFreq);
        addAndMakeVisible (bandwidth);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        int label_height = juce::roundToInt (b.getHeight() * 0.2f);
        label.setBounds( b.removeFromTop (label_height));
            
        auto unitWidth = b.getWidth() / static_cast<float> (2);
        centerFreq.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
        bandwidth.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
    }
private:
    morph::AutoFitTextBox label;
    KnobParameterSlider centerFreq, bandwidth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandpassPanel)
};

class TrajectoryVariables : public juce::Component 
{
public:
    TrajectoryVariables (juce::AudioProcessorValueTreeState& vts, 
                         tp::MPEVoiceData& vd)
      : amplitude ("Amplitude", "Amplitude", vts, vd),
        pan ("Pan", "Pan", vts, vd),
        pitch ("Pitch", "Pitch", vts, vd),
        cents ("Cents", "Cents", vts, vd),
        size ("Size", "Size", vts, vd),
        rotation ("Rotation", "Rotation", vts, vd),
        translation_x ("X", "TranslationX", vts, vd),
        translation_y ("Y", "TranslationY", vts, vd)
    {
        addAndMakeVisible (amplitude);
        addAndMakeVisible (pan);
        addAndMakeVisible (pitch);
        addAndMakeVisible (size);
        addAndMakeVisible (cents);
        addAndMakeVisible (rotation);
        addAndMakeVisible (translation_x);
        addAndMakeVisible (translation_y);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        auto upperBounds = b.removeFromTop( b.getHeight() / 2 );
        auto lowerBounds = b;

        auto unitWidth = juce::roundToInt (b.getWidth() / static_cast<float> (4));

        amplitude.setBounds (upperBounds.removeFromLeft (static_cast<int> (unitWidth)));
        pan.setBounds (upperBounds.removeFromLeft (static_cast<int> (unitWidth)));
        pitch.setBounds (upperBounds.removeFromLeft( static_cast<int> (unitWidth)));
        cents.setBounds (upperBounds.removeFromLeft( static_cast<int> (unitWidth)));


        size.setBounds (lowerBounds.removeFromLeft (static_cast<int> (unitWidth)));
        rotation.setBounds (lowerBounds.removeFromLeft (static_cast<int> (unitWidth)));
        translation_x.setBounds (lowerBounds.removeFromLeft (static_cast<int> (unitWidth)));
        translation_y.setBounds (lowerBounds.removeFromLeft (static_cast<int> (unitWidth)));
    }
private:
    KnobParameterSlider amplitude;
    KnobParameterSlider pan;
    KnobParameterSlider pitch;
    KnobParameterSlider cents;
    KnobParameterSlider size;
    KnobParameterSlider rotation;
    KnobParameterSlider translation_x;
    KnobParameterSlider translation_y;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryVariables)
};
class MeanderancePanel : public juce::Component
{
public:
    MeanderancePanel (juce::AudioProcessorValueTreeState& vts,
                      tp::MPEVoiceData& vd)
      : scale ("Scale", "MeanderanceScale", vts, vd),
        speed ("Speed", "MeanderanceSpeed", vts, vd),
        caffiene ("Caffiene", "MeanderanceCaffiene", vts, vd)
    {
        label.setText ("Meanderance", juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        addAndMakeVisible (scale);
        addAndMakeVisible (speed);
        addAndMakeVisible (caffiene);
    }
    void resized()
    {
        auto b = getLocalBounds();

        auto text_height = juce::roundToInt (juce::roundToInt (b.getHeight() * 0.2f));
        label.setBounds (b.removeFromTop (text_height));
        
        auto unitWidth = juce::roundToInt (b.getWidth() / static_cast<float> (3));
        scale.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
        speed.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
        caffiene.setBounds (b.removeFromLeft (static_cast<int> (unitWidth)));
    }
private:
    morph::AutoFitTextBox label;
    KnobParameterSlider scale, speed, caffiene;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeanderancePanel)
};



class TrajectoryPanel : public Panel
{
public:
    TrajectoryPanel (juce::AudioProcessorValueTreeState& vts, 
                     tp::MPEVoiceData& vd)
      : Panel ("Trajectory"),  
        trajectorySelector (vts, vd),
        trajectoryVariables (vts, vd),
        meanderancePanel (vts, vd),
        feedbackPanel (vts, vd),
        radialCompressorPanel (vts, vd),
        bandPassPanel (vts, vd)
    {
        addAndMakeVisible (trajectorySelector);
        addAndMakeVisible (trajectoryVariables);
        addAndMakeVisible (meanderancePanel);
        addAndMakeVisible (feedbackPanel);
        addAndMakeVisible (radialCompressorPanel);
        addAndMakeVisible (bandPassPanel);

        for( int i = 0; i < 6; i++) { lines.add (std::make_unique<Line> (lineThickness));}
        for( auto* line : lines ) { addAndMakeVisible (line); }
    }
    void resized () override 
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        auto unitHeight = juce::roundToInt ( (b.getHeight() - lines.size() * lineThickness) / static_cast<float> (7));

        jassert (lines.size() >= 6);
        lines.getUnchecked( 0 )->setBounds (b.removeFromTop (lineThickness));
        trajectorySelector.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        lines.getUnchecked( 1 )->setBounds (b.removeFromTop (lineThickness));
        trajectoryVariables.setBounds (b.removeFromTop (static_cast<int> (unitHeight * 2)));
        lines.getUnchecked( 2 )->setBounds (b.removeFromTop (lineThickness));
        meanderancePanel.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        lines.getUnchecked( 3 )->setBounds (b.removeFromTop (lineThickness));
        feedbackPanel.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        lines.getUnchecked( 4 )->setBounds (b.removeFromTop (lineThickness));
        radialCompressorPanel.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        lines.getUnchecked( 5 )->setBounds (b.removeFromTop (lineThickness));
        bandPassPanel.setBounds (b.removeFromTop (static_cast<int> (unitHeight)));
        
    }
private:
    TrajectorySelector trajectorySelector;
    TrajectoryVariables trajectoryVariables;
    MeanderancePanel meanderancePanel;
    FeedbackPanel feedbackPanel;
    RadialCompressorPanel radialCompressorPanel;
    BandpassPanel bandPassPanel;

    struct Line : public juce::Component {
        explicit Line (int w = 2) : thickness (static_cast<float>(w)) {}
        void paint (juce::Graphics& g) {
            auto bounds = getLocalBounds();
            auto* tlaf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel() );
            jassert (tlaf != nullptr);
            g.setColour (tlaf->getBackgroundDark().darker());
            g.drawLine (0.0f, 0.0f, bounds.toFloat().getWidth(), 0.0, thickness);       
        }
        float thickness { 2.0f };
    };
    juce::OwnedArray<Line> lines;
    const int lineThickness { 6 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryPanel)
};
}