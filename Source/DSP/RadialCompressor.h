#pragma once 

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace tp 
{

class RadialCompressor
{
public:
    void reset();
    void prepare (double sampleRate);

    juce::Point<float> processPoint (const juce::Point<float> input);
    void setThreshold (float threshold); // 0.0 to 1.0, think in terms of radius
    void setRatio (float newRatio); // 1.0f to infinity (and beyond)
    void setKnee (float newKnee); // 0.0f hard knee, threshold is the maximum reasonable value. 0.1 is the default for Terrain
    void setResponsiveness (float newResponsivenessMS); // How fast does the compressor react to change? Must be above 0.0f MS
private:
    float threshold { 1.0f };
    float ratio { 3.0f };
    float knee { 0.0f }; // 0.0 is hard knee
    float alpha { 0.02f };
    float averageRadius { 0.0f };
    double sampleRate { -1.0 };
    float computeGain (float inputRadius);
};

}