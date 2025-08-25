#pragma once 

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <PerlinNoise/PerlinNoise.hpp>

namespace tp
{

struct PerlinVector
{
    PerlinVector() 
    {
        juce::Random r;
        r.setSeedRandomly();
        noiseX.reseed (static_cast<unsigned int> (r.nextInt()));
        noiseY.reseed (static_cast<unsigned int> (r.nextInt()));
        
        phase = 0.0;
        phaseIncrement = 0.005;
        sampleIndex = 0;
        sampleInterval = 1024;
        smoothX.reset (sampleInterval);
        smoothY.reset (sampleInterval);
    }
    juce::Point<float> getNext()
    {
        incrementSampleIndex();
        return {smoothX.getNextValue(), smoothY.getNextValue()};
    }
    void setSpeed (double newSpeed) // 0 - 1 expected (arbitrary decision)
    {
        phaseIncrement = newSpeed * inverseSampleRate * 1500.0;
    }
    void setSampleRate (double newSampleRate)
    {
        // sampleInterval = static_cast<int> (newSampleRate * (48000.0 / 512.0));
        inverseSampleRate = 1.0 / newSampleRate;
    }
    private:
    siv::BasicPerlinNoise<float> noiseX, noiseY;
    juce::SmoothedValue<float> smoothX, smoothY;
    double inverseSampleRate = 1.0 / 48000.0;
    double phase, phaseIncrement;
    int sampleInterval, sampleIndex;

    void incrementSampleIndex()
    {
        sampleIndex++;
        if (sampleIndex >= sampleInterval)
        {
            phase += phaseIncrement;
            smoothX.setTargetValue (noiseX.noise1D (static_cast<float>(phase)));
            smoothY.setTargetValue (noiseY.noise1D (static_cast<float>(phase)));
            sampleIndex -= sampleInterval;
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerlinVector)
};

class Meanderer
{
public:
    void prepare (double sr);
    juce::Point<float> process (const juce::Point<float> input);
    void setScale (float newScale); // 0.0 to 1.0
    void setSpeed (float newSpeed); // 0.0 to 1.0
    void setCaffiene (float newCaffiene); // 0.0 to 1.0
private:
    PerlinVector position;
    PerlinVector caffieneVector;
    siv::BasicPerlinNoise<float> caffieneSpeed;
    float caffiene {0.0f};
    float caffienePhase {0.0f};
    float caffienePhaseInc {0.0f};
    double inverseSampleRate {-1.0};

    float scale {1.0f};
    void incrementCaffiene();
};
}