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
struct Parameters
{
    ChoiceParameter* currentTrajectoryParameter;
};
}
