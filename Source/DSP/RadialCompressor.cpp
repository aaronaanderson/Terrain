#include "RadialCompressor.h"

#include <juce_core/juce_core.h>
using namespace tp;

void RadialCompressor::reset() 
{
	averageRadius = 0.0f;
}
void RadialCompressor::prepare (double newSampleRate, int maxBlockSize) 
{ 
    sampleRate = newSampleRate;
}
juce::Point<float> RadialCompressor::processPoint (const juce::Point<float> input) 
{ 
	juce::Point<float> output {0.0f, 0.0f};
	
    // Use hypot for stable radius computation (avoids overflow/underflow issues).
    const float inputRadius = std::hypot( output.x, output.y );

    // Update average radius (exponential moving average on incoming radius).
    averageRadius += alpha * (inputRadius - averageRadius);

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
    /*
        // I'm going to be completely honest, I vibe coded this bit
        // with ChatGPT.  I've left the comments in, but I don't totally
        // understand why this works. -AA
    */
    // Guard: if input radius is zero/negative, no scaling needed.
    if (inputRadius <= 0.0f)
        return 1.0f;


    // Hard-knee target output level (what we'd want Lin to become).
    float outHard;
    if (inputRadius <= threshold)
        outHard = inputRadius;                          // below threshold: passthrough
    else
        outHard = threshold + (inputRadius - threshold) / ratio;        // above threshold: compress excess by ratio

    // If no soft knee, just return hard-knee gain.
    if (knee <= 0.0f)
        return juce::jlimit (0.0f, 1.0f, outHard / inputRadius);

    // Soft-knee: linearly crossfade gain from unity to hard-knee over [T - k/2, T + k/2].
    const float halfK = 0.5f * knee;
    const float lo = juce::jmax (0.0f, threshold - halfK);
    const float hi = threshold + halfK;

    if (inputRadius <= lo)
        return 1.0f; // fully unity below knee band

    if (inputRadius >= hi)
        return juce::jlimit (0.0f, 1.0f, outHard / inputRadius); // fully hard-knee above knee band

    // Inside knee band: linear crossfade between unity and hard-knee gain.
    const float t   = (inputRadius - lo) / (hi - lo);             // 0..1
    const float g1  = 1.0f;                                // unity gain
    const float gHK = juce::jlimit (0.0f, 1.0f, outHard / inputRadius);
    return (1.0f - t) * g1 + t * gHK;
}