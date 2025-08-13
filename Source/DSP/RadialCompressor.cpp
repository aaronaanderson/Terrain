#include "RadialCompressor.h"

#include <juce_core/juce_core.h>
using namespace tp;

void RadialCompressor::reset() 
{
	averageRadius = 0.0f;
}
void RadialCompressor::prepare (double newSampleRate) 
{ 
    sampleRate = newSampleRate;
}
juce::Point<float> RadialCompressor::processPoint (const juce::Point<float> input) 
{ 
    // Use hypot for stable radius computation (avoids overflow/underflow issues).
    //const float inputRadius = std::hypot( input.x, input.y );
    const float inputRadius = input.getDistanceFromOrigin();
    jassert (inputRadius >= 0.0f);
    // Update average radius (exponential moving average on incoming radius).
    averageRadius += alpha * (inputRadius - averageRadius);
    jassert (averageRadius >= 0.0f);

    const float gain = computeGain (averageRadius);

    return { input.x * gain, input.y * gain };
}
void RadialCompressor::setThreshold (float newThreshold) { threshold = newThreshold; }
void RadialCompressor::setRatio (float newRatio) { ratio = newRatio; }
void RadialCompressor::setKnee (float newKnee) { knee = newKnee; }
void RadialCompressor::setResponsiveness (float newResponsivenesMS)
{
    jassert (sampleRate > 0.0);
    jassert (newResponsivenesMS > 0.0f);

    const float windowSamples = static_cast<float>(sampleRate) * newResponsivenesMS * 0.001f;
    // map to EMA alpha
    alpha = 1.0f - std::exp (-1.0f / windowSamples);
}
float RadialCompressor::computeGain (float inputRadius)
{
    jassert (inputRadius >= 0.0f);

    if (inputRadius < 1.0e-6f) // near origin, leave it alone
        return 1.0f;

    // Hard knee
    if (inputRadius <= threshold)
        return 1.0f;

    const float excess      = inputRadius - threshold;
    const float compressed  = excess / ratio;
    const float target      = threshold + compressed;
    const float gain        = target / inputRadius;

    return gain; //juce::jlimit (0.0f, 1.0f, gain);
}