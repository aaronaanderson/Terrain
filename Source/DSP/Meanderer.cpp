#include "Meanderer.h"

using namespace tp;

void Meanderer::prepare (double sr)
{
	position.setSampleRate (sr);
	inverseSampleRate = 1.0 / sr;
}

juce::Point<float> Meanderer::process (const juce::Point<float> input)
{
	jassert (inverseSampleRate >= 0.0 && inverseSampleRate <= 1.0);

	juce::Point<float> output {input};

	output += position.getNext() * scale;

	output += caffieneVector.getNext() * caffiene * 0.333f;

	incrementCaffiene();

	return output;
}

void Meanderer::setScale (float newScale)
{
	jassert (newScale >= 0.0f);
	scale = newScale;
}

void Meanderer::setSpeed (float newSpeed)
{
	jassert (newSpeed >= 0.0f && newSpeed <= 1.0f);
	position.setSpeed (newSpeed);
}

void Meanderer::setCaffiene (float newCaffiene)
{
	juce::jlimit (0.0f, 1.0f, newCaffiene);
	caffiene = newCaffiene;
}

void Meanderer::incrementCaffiene()
{
    caffienePhase += caffienePhaseInc;
	auto speed = caffieneSpeed.noise1D (caffienePhase);
	caffieneVector.setSpeed (speed * 16.0f * caffiene);
}