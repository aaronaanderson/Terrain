#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
namespace tp
{ 
class ChoiceParameter : public juce::AudioParameterChoice
{
public:
    ChoiceParameter(juce::String parameterName, 
                    juce::StringArray iChoices, 
                    juce::String unit,
                    int defaultChoice = 0)
      : juce::AudioParameterChoice ({parameterName.removeCharacters(" ")/* + juce::String("Choice")*/, 1}, 
                                    parameterName, 
                                    iChoices, 
                                    defaultChoice,
                                    juce::AudioParameterChoiceAttributes().withLabel (unit)),
        options (iChoices)
    {
        valueChanged (defaultChoice);
    }

    void setIndex (int newIndex)
    {
        auto normalized = newIndex / static_cast<float> (options.size() - 1);
        setValueNotifyingHost (normalized);
    }
private: 
    juce::StringArray options;
};
class NormalizedFloatParameter : public juce::AudioParameterFloat
{
public:
    NormalizedFloatParameter (juce::String parameterName, 
                              float defaultValue = 0.0f,
                              juce::String unit = "")
    : juce::AudioParameterFloat ({parameterName.removeCharacters(" ")/* + juce::String("float")*/, 1}, 
                                 parameterName,
                                 {0.0f, 1.0f},  
                                 defaultValue,
                                 juce::AudioParameterFloatAttributes().withLabel (unit)
                                                                       .withStringFromValueFunction ([&](float v, int n){ juce::ignoreUnused (n); return juce::String (v, 3); }))
    {
        valueChanged (defaultValue);
    }
};
class RangedFloatParameter : public juce::AudioParameterFloat
{
public:
    RangedFloatParameter (juce::String parameterName, 
                          juce::NormalisableRange<float> pRange,
                          float defaultValue = 0.0f,
                          juce::String unit = "")
    : juce::AudioParameterFloat ({parameterName.removeCharacters(" ")/* + juce::String("float")*/, 1}, 
                                 parameterName,
                                 pRange,  
                                 defaultValue,
                                 juce::AudioParameterFloatAttributes().withLabel (unit)
                                                                      .withStringFromValueFunction ([&](float v, int n){ juce::ignoreUnused (n); return juce::String (v, 3); }))
    {
        valueChanged (defaultValue);
    }
};
struct Parameters
{
    Parameters (juce::AudioProcessorValueTreeState& vts)
      : valueTreeState (vts)
    {}
private:
    juce::AudioProcessorValueTreeState& valueTreeState;
public:
    ChoiceParameter* currentTrajectory              = dynamic_cast<ChoiceParameter*> (valueTreeState.getParameter ("CurrentTrajectory"));
    NormalizedFloatParameter* trajectoryModA        = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter ("TrajectoryModA"));
    NormalizedFloatParameter* trajectoryModB        = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter ("TrajectoryModB"));
    NormalizedFloatParameter* trajectoryModC        = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter ("TrajectoryModC"));
    NormalizedFloatParameter* trajectoryModD        = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter ("TrajectoryModD"));
    
    NormalizedFloatParameter* trajectoryAmplitude = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter     ("Amplitude")); 
    NormalizedFloatParameter* trajectorySize = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter     ("Size"));       
    RangedFloatParameter*     trajectoryRotation = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter     ("Rotation"));
    RangedFloatParameter*     trajectoryTranslationX = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter ("TranslationX"));
    RangedFloatParameter*     trajectoryTranslationY = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter ("TranslationY"));
    NormalizedFloatParameter* meanderanceScale = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter   ("MeanderanceScale"));
    NormalizedFloatParameter*     meanderanceSpeed = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter       ("MeanderanceSpeed"));

    RangedFloatParameter*     feedbackTime = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter        ("FeedbackTime"));
    RangedFloatParameter*     feedbackScalar = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter      ("Feedback"));
    RangedFloatParameter*     feedbackCompression = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter ("FeedbackCompression"));
    NormalizedFloatParameter*     feedbackMix = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter         ("FeedbackMix"));

    ChoiceParameter* currentTerrain = dynamic_cast<ChoiceParameter*> (valueTreeState.getParameter                ("CurrentTerrain"));
    NormalizedFloatParameter* terrainModA = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter ("TerrainModA"));
    NormalizedFloatParameter* terrainModB = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter ("TerrainModB"));
    NormalizedFloatParameter* terrainModC = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter ("TerrainModC"));
    NormalizedFloatParameter* terrainModD = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter ("TerrainModD"));

    RangedFloatParameter* terrainSaturation = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter ("TerrainSaturation"));

    juce::AudioParameterBool* envelopeSize = dynamic_cast<juce::AudioParameterBool*> (valueTreeState.getParameter ("EnvelopeSize"));
    RangedFloatParameter* velocity = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter               ("Velocity"));
    RangedFloatParameter* attack = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter               ("Attack"));
    RangedFloatParameter* decay = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter                ("Decay"));
    RangedFloatParameter* sustain = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter              ("Sustain"));
    RangedFloatParameter* release = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter              ("Release"));

    NormalizedFloatParameter* filterResonance = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter ("FilterResonance"));
    RangedFloatParameter*     filterFrequency = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter     ("FilterFrequency"));
    juce::AudioParameterBool* filterOnOff = dynamic_cast<juce::AudioParameterBool*> (valueTreeState.getParameter     ("FilterOnOff"));

    NormalizedFloatParameter* perVoiceFilterResonance = dynamic_cast<NormalizedFloatParameter*> (valueTreeState.getParameter ("Per-VoiceFilterResonance"));
    RangedFloatParameter*     perVoiceFilterFrequency = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter     ("Per-VoiceFilterFrequency"));
    juce::AudioParameterBool* perVoiceFilterOnOff = dynamic_cast<juce::AudioParameterBool*> (valueTreeState.getParameter     ("Per-VoiceFilterOnOff"));

    RangedFloatParameter*     compressorThreshold = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter ("CompressorThreshold"));
    RangedFloatParameter*     compressorRatio = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter     ("CompressorRatio"));

    RangedFloatParameter*     outputLevel = dynamic_cast<RangedFloatParameter*> (valueTreeState.getParameter("OutputLevel"));
};
}
