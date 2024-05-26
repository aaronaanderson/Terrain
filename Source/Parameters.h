#pragma once

namespace tp
{
class ChoiceParameter : public juce::AudioParameterChoice
{
public:
    ChoiceParameter(juce::String parameterName, 
                    juce::StringArray iChoices, 
                    juce::String label,
                    std::function<void(int)> newValueFunction = nullptr, 
                    int defaultChoice = 0)
      : choices (iChoices), 
        onNewValue (newValueFunction),
        juce::AudioParameterChoice (parameterName.removeCharacters(" ") + juce::String("Choice"), 
                                    parameterName, 
                                    iChoices, 
                                    defaultChoice,
                                    juce::AudioParameterChoiceAttributes().withLabel (label))
    {
        valueChanged (defaultChoice);
    }
    void valueChanged(int newValue) override
    {
        if (onNewValue == nullptr) return;
        if (newValue == storedValue) return;
        onNewValue (newValue);
        storedValue = newValue;
    }
    void setIndex (int newIndex)
    {
        auto normalized = newIndex / static_cast<float> (choices.size() - 1);
        setValueNotifyingHost (normalized);
    }
private: 
    std::function<void(int)> onNewValue;
    juce::StringArray choices;
    int storedValue = -1;
};
class NormalizedFloatParameter : public juce::AudioParameterFloat
{
public:
    NormalizedFloatParameter (juce::String parameterName, 
                              float defaultValue = 0.0f,
                              juce::String label = "", 
                              std::function<void(float)> newValueFunction = nullptr)
    : onNewValue (newValueFunction), 
      juce::AudioParameterFloat (parameterName.removeCharacters(" ") + juce::String("Choice"), 
                                  parameterName,
                                  {0.0f, 1.0f},  
                                  defaultValue,
                                  juce::AudioParameterFloatAttributes().withLabel (label)
                                                                       .withStringFromValueFunction ([&](float v, int n){ juce::ignoreUnused (n); return juce::String (v, 3); }))
    {
        valueChanged (defaultValue);
    }

private:
    void valueChanged (float newValue) override 
    {
        if (onNewValue != nullptr) 
            onNewValue (newValue);
    }
    std::function<void(float)> onNewValue;
};

class RangedFloatParameter : public juce::AudioParameterFloat
{
public:
    RangedFloatParameter (juce::String parameterName, 
                          juce::NormalisableRange<float> range,
                              float defaultValue = 0.0f,
                              juce::String label = "", 
                              std::function<void(float)> newValueFunction = nullptr)
    : onNewValue (newValueFunction), 
      juce::AudioParameterFloat (parameterName.removeCharacters(" ") + juce::String("Choice"), 
                                  parameterName,
                                  range,  
                                  defaultValue,
                                  juce::AudioParameterFloatAttributes().withLabel (label)
                                                                       .withStringFromValueFunction ([&](float v, int n){ juce::ignoreUnused (n); return juce::String (v, 3); }))
    {
        valueChanged (defaultValue);
    }

private:
    void valueChanged (float newValue) override 
    {
        if (onNewValue != nullptr) 
            onNewValue (newValue);
    }
    std::function<void(float)> onNewValue;
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

    ChoiceParameter* currentTerrain;
    NormalizedFloatParameter* terrainModA;
    NormalizedFloatParameter* terrainModB;
    NormalizedFloatParameter* terrainModC;
    NormalizedFloatParameter* terrainModD;
};
}
