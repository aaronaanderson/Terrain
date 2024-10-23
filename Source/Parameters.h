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
      : juce::AudioParameterChoice ({parameterName.removeCharacters(" ") + juce::String("Choice"), 1}, 
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
    : juce::AudioParameterFloat ({parameterName.removeCharacters(" ") + juce::String("float"), 1}, 
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
    : juce::AudioParameterFloat ({parameterName.removeCharacters(" ") + juce::String("float"), 1}, 
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
    NormalizedFloatParameter* meanderanceScale;
    RangedFloatParameter*     meanderanceSpeed;


    RangedFloatParameter*     feedbackTime;
    RangedFloatParameter*     feedbackScalar;
    RangedFloatParameter*     feedbackCompression;
    RangedFloatParameter*     feedbackMix;

    ChoiceParameter* currentTerrain;
    NormalizedFloatParameter* terrainModA;
    NormalizedFloatParameter* terrainModB;
    NormalizedFloatParameter* terrainModC;
    NormalizedFloatParameter* terrainModD;

    RangedFloatParameter*     terrainSaturation;

    juce::AudioParameterBool* envelopeSize;
    RangedFloatParameter* attack;
    RangedFloatParameter* decay;
    RangedFloatParameter* sustain;
    RangedFloatParameter* release;

    NormalizedFloatParameter* filterResonance;
    RangedFloatParameter*     filterFrequency;
    juce::AudioParameterBool* filterOnOff;

    RangedFloatParameter*     compressorThreshold;
    RangedFloatParameter*     compressorRatio;

    RangedFloatParameter*     outputLevel;
};
}
