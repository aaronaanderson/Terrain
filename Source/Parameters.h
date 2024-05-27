#pragma once

namespace tp
{
class ChoiceParameter : public juce::AudioParameterChoice
{
public:
    ChoiceParameter(juce::String parameterName, 
                    juce::StringArray iChoices, 
                    juce::String unit,
                    int defaultChoice = 0)
      : juce::AudioParameterChoice (parameterName.removeCharacters(" ") + juce::String("Choice"), 
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
    : juce::AudioParameterFloat (parameterName.removeCharacters(" ") + juce::String("Choice"), 
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
    : juce::AudioParameterFloat (parameterName.removeCharacters(" ") + juce::String("Choice"), 
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
    ChoiceParameter* currentTrajectory;
    NormalizedFloatParameter* trajectoryModA;
    NormalizedFloatParameter* trajectoryModB;
    NormalizedFloatParameter* trajectoryModC;
    NormalizedFloatParameter* trajectoryModD;

    NormalizedFloatParameter* trajectorySize;
    RangedFloatParameter*     trajectoryRotation;
    RangedFloatParameter*     trajectoryTranslationX;
    RangedFloatParameter*     trajectoryTranslationY;

    RangedFloatParameter*     feedbackTime;
    RangedFloatParameter*     feedbackScalar;
    RangedFloatParameter*     feedbackMix;

    ChoiceParameter* currentTerrain;
    NormalizedFloatParameter* terrainModA;
    NormalizedFloatParameter* terrainModB;
    NormalizedFloatParameter* terrainModC;
    NormalizedFloatParameter* terrainModD;
};
}
