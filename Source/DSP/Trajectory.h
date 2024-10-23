#pragma once 

#include <juce_audio_basics/juce_audio_basics.h>
#include "../Parameters.h"
#include "DataTypes.h"
#include "ADSR.h"
#include "Terrain.h"

namespace tp{
static float distance (const Point a, const Point b)
{
    return static_cast<float> (std::sqrt (std::pow (a.x - b.x, 2) + std::pow (a.y - b.y, 2)));
}
static Point normalize (const Point p, const float n = 1.0f)
{
    auto d = distance (p, Point (0.0f, 0.0f));
    auto adjustmentScalar = n / d;
    return Point (p.x * adjustmentScalar, p.y * adjustmentScalar);
}

class Trajectory : public juce::SynthesiserVoice
{
public:
    Trajectory (Parameters& p)
      : voiceParameters (p)
    {
        envelope.prepare (sampleRate);
        envelope.setParameters ({200.0f, 20.0f, 0.7f, 1000.0f});
        functions = 
        {
            [&](float theta, ModSet m){ return Point (std::sin(theta) * m.a, std::cos(theta));} 
            ,[&](float theta, ModSet m)
                {
                    auto n = std::pow (m.a, 2.0f) * 5 + 0.5f;
                    auto a = m.b * 0.5f + 0.5f;
                    auto b = m.c * 0.5f + 0.5f;
                    auto r = std::pow (std::pow (std::abs (std::cos (theta) / a), n) + std::pow (std::abs (std::sin (theta) / b), n), (-1.0f / n));
                    return Point (r * std::cos (theta), r * std::sin (theta));
                }
            ,[&](float theta, ModSet m)
                {   float r = m.b + m.a * std::sin (theta);
                    return Point(r * std::cos(theta), r * std::sin(theta)); }
            ,[&](float theta, ModSet m)
                {   float r = powf (juce::MathConstants<float>::euler, std::cos (theta + (m.a * juce::MathConstants<float>::twoPi)))
                              - 2.0f * std::cos (4.0f * theta) 
                              + powf (std::sin((2.0f * theta - juce::MathConstants<float>::pi) / 24.0f), 5);
                    return Point(r * std::cos(theta), r * std::sin(theta)); }
            ,[&](float theta, ModSet m)
                {   float r = (m.b * std::cos (2.0f * theta) - m.a * std::cos (theta));
                    return Point(r * std::cos(theta), r * std::sin(theta)); }
            // Squarcle
            ,[&](float theta, ModSet m) { return Point (std::tanh (std::sin (theta) * (m.a * 3.0f + 1.0f) ), 
                                                        std::tanh (std::cos (theta) * (m.a * 3.0f + 1.0f) )); }
            // Bicorn
            ,[&](float theta, ModSet m) 
                {   juce::ignoreUnused (m); 
                    return Point (std::sin (theta), 
                                   ((2.0f + std::cos (theta)) * std::pow (std::cos (theta), 2.0f)) / 
                                    (3.0f + std::pow (std::sin (theta), 2.0f))); }
            // Cornoid
            ,[&](float theta, ModSet m) 
                {   auto aa = m.a * 2.0f + 0.01f;
                    return Point (std::cos (theta) * std::cos (2.0f * theta),
                                  juce::jmap (aa, 0.01f, 2.01f, 1.0f, 0.5f) * std::sin (theta) * (aa + std::cos (2.0f * theta))); }
            // Epitrochoid 3
            ,[&](float theta, ModSet m) 
                {    
                    auto d = m.a + 0.01f;
                    auto r = (1.0f - d) / 4.0f;
                    auto R = 3.0f * r;
                    return Point (((R + r) * std::cos (theta)) - (d * std::cos (((R + r) / r) * theta)), 
                                  ((R + r) * std::sin (theta)) - (d * std::sin (((R + r) / r) * theta)));
                }
            // Epitrochoid 5
            ,[&](float theta, ModSet m) 
                {    
                    auto d = m.a + 0.01f;
                    auto r = (1.0f - d) / 6.0f;
                    auto R = 5.0f * r;
                    return Point (((R + r) * std::cos (theta)) - (d * std::cos (((R + r) / r) * theta)), 
                                  ((R + r) * std::sin (theta)) - (d * std::sin (((R + r) / r) * theta)));
                }
            // Epitrochoid 7
            ,[&](float theta, ModSet m) 
                {    
                    auto d = m.a + 0.01f;
                    auto r = (1.0f - d) / 8.0f;
                    auto R = 7.0f * r;
                    return Point (((R + r) * std::cos (theta)) - (d * std::cos (((R + r) / r) * theta)), 
                                  ((R + r) * std::sin (theta)) - (d * std::sin (((R + r) / r) * theta)));
                }
            // Hypocycloid 3
            ,[&](float theta, ModSet m) 
            {    
                auto R = 1.0f;
                auto r = R / 3.0f;
                return Point (((R - r) * std::cos (theta)) + (m.a * r * std::cos (((R - r) / r) * theta)), 
                              ((R - r) * std::sin (theta)) - (m.a * r * std::sin (((R - r) / r) * theta)));
            }
            // Hypocycloid 5
            ,[&](float theta, ModSet m) 
            {    
                auto R = 1.0f;
                auto r = R / 5.0f;
                return Point (((R - r) * std::cos (theta)) + (m.a * r * std::cos (((R - r) / r) * theta)), 
                              ((R - r) * std::sin (theta)) - (m.a * r * std::sin (((R - r) / r) * theta)));
            }
            // Hypocycloid 7
            ,[&](float theta, ModSet m) 
            {    
                auto R = 1.0f;
                auto r = R / 7.0f;
                return Point (((R - r) * std::cos (theta)) + (m.a * r * std::cos (((R - r) / r) * theta)), 
                              ((R - r) * std::sin (theta)) - (m.a * r * std::sin (((R - r) / r) * theta)));
            }
            // Gear Curve 3
            ,[&](float theta, ModSet m) 
            {
                auto b = (10.0f - m.a * 10.0f) + 2.0f;
                auto r = 1.0f + ((1.0f / b) * std::tanh (b * std::sin (3 * theta)));
                return Point (r * std::cos (theta), r * sin (theta));
            }
            // Gear Curve 5
            ,[&](float theta, ModSet m) 
            {
                auto b = (10.0f - m.a * 10.0f) + 2.0f;
                auto r = 1.0f + ((1.0f / b) * std::tanh (b * std::sin (5 * theta)));
                return Point (r * std::cos (theta), r * sin (theta));
            }
            // Gear Curve 7
            ,[&](float theta, ModSet m) 
            {
                auto b = (10.0f - m.a * 10.0f) + 2.0f;
                auto r = 1.0f + ((1.0f / b) * std::tanh (b * std::sin (7 * theta)));
                return Point (r * std::cos (theta), r * sin (theta));
            }
        };
    }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<Terrain*>(s) != nullptr; }
    void startNote (int midiNoteNumber,
                    float velocity,
                    juce::SynthesiserSound* sound,
                    int currentPitchWheelPosition) override 
    {
        juce::ignoreUnused (currentPitchWheelPosition);
        
        setFrequency (static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber)));
        amplitude = velocity;
        terrain = dynamic_cast<Terrain*> (sound);
        envelope.noteOn();
        voiceParameters.noteOn();
        feedbackBuffer.fill (Point(0.0f, 0.0f));
    }
    void stopNote (float velocity, bool allowTailOff) override 
    { 
        juce::ignoreUnused (velocity, allowTailOff); 
        envelope.noteOff();
    }
    void pitchWheelMoved (int newPitchWheelValue) override { juce::ignoreUnused (newPitchWheelValue); }
    void controllerMoved (int controllerNumber, int newControllerValue) override { juce::ignoreUnused (controllerNumber, newControllerValue); }
    void renderNextBlock (juce::AudioBuffer<double>& ob, int ss, int nums) override { juce::ignoreUnused (ob, ss, nums); }
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, 
                          int startSample, int numSamples) override 
    {
         auto* o = outputBuffer.getWritePointer(0);
        for(int i = startSample; i < startSample + numSamples; i++)
        {
            if(!envelope.isActive()) break;
            tp::ADSR::Parameters p = {voiceParameters.attack.getNext(), 
                                      voiceParameters.decay.getNext(), 
                                      voiceParameters.sustain.getNext(), 
                                      voiceParameters.release.getNext()};
            envelope.setParameters (p);

            auto point = functions[*voiceParameters.currentTrajectory](static_cast<float> (phase), getModSet());
            
            point = rotate (point, voiceParameters.rotation.getNext());
            point = scale (point, voiceParameters.size.getNext() * amplitude);
            if (*voiceParameters.envelopeSize)
                point = scale (point, static_cast<float> (envelope.getCurrentValue()));
            point = feedback (point, 
                              voiceParameters.feedbackTime.getNext(), 
                              voiceParameters.feedbackScalar.getNext(), 
                              voiceParameters.feedbackMix.getNext(), 
                              voiceParameters.size.getCurrent(), 
                              voiceParameters.feedbackCompression.getNext());
            point = translate (point, 
                               voiceParameters.translationX.getNext(), 
                               voiceParameters.translationY.getNext());
            perlinVector.setSpeed (voiceParameters.meanderanceSpeed.getNext());
            point = meander (point, voiceParameters.meanderanceScale.getNext());
            point = compressEdge (point);

            if (terrain != nullptr)
            {
                float outputSample = terrain->sampleAt (point, i);
                history.feedNext (point, outputSample);
                o[i] += outputSample * static_cast<float> (envelope.calculateNext());
            }
            phase = std::fmod (phase + phaseIncrement, juce::MathConstants<double>::twoPi);
            if(!envelope.isActive())
            {
                history.clear();
                clearCurrentNote();
            }
        }
    } 
    void setCurrentPlaybackSampleRate (double newRate) override 
    {
        if (newRate > 0.0)
        {
            sampleRate = newRate;
            envelope.prepare (sampleRate);
            setFrequency (frequency);
            perlinVector.setSampleRate (newRate);
        }
        // two second max delay
        feedbackBuffer.resize (static_cast<int> (sampleRate) * 2);
        feedbackBuffer.fill (Point(0.0f, 0.0f));
    }
    void prepareToPlay (double newRate, int blockSize)
    {
        juce::ignoreUnused (blockSize);
        voiceParameters.resetSampleRate (newRate);
    }
    const float* getRawData() { return history.getRawData(); }
private:
    ADSR envelope;
    Terrain* terrain;
    juce::Array<std::function<Point(float, ModSet)>> functions;
    struct VoiceParameters
    {
        VoiceParameters (Parameters& p)
          : currentTrajectory (p.currentTrajectory),
            mod_a (p.trajectoryModA),
            mod_b (p.trajectoryModB),
            mod_c (p.trajectoryModC),
            mod_d (p.trajectoryModD), 
            size (p.trajectorySize), 
            rotation (p.trajectoryRotation), 
            translationX (p.trajectoryTranslationX), 
            translationY (p.trajectoryTranslationY), 
            meanderanceScale (p.meanderanceScale),
            meanderanceSpeed (p.meanderanceSpeed),
            feedbackScalar (p.feedbackScalar), 
            feedbackTime (p.feedbackTime), 
            feedbackCompression (p.feedbackCompression),
            feedbackMix (p.feedbackMix), 
            envelopeSize (p.envelopeSize),
            attack (p.attack), 
            decay (p.decay), 
            sustain (p.sustain), 
            release (p.release)
        {}
        void noteOn()
        {
            mod_a.noteOn();
            mod_b.noteOn();
            mod_c.noteOn();
            mod_d.noteOn();
            size.noteOn();
            rotation.noteOn();
            translationX.noteOn();
            translationY.noteOn();
            meanderanceScale.noteOn();
            meanderanceSpeed.noteOn();
            feedbackScalar.noteOn();
            feedbackTime.noteOn();
            feedbackCompression.noteOn();
            feedbackMix.noteOn();
            attack.noteOn();
            decay.noteOn();
            sustain.noteOn();
            release.noteOn();
        }
        void resetSampleRate (double newSampleRate)
        {
            mod_a.prepare (newSampleRate);
            mod_b.prepare (newSampleRate);
            mod_c.prepare (newSampleRate);
            mod_d.prepare (newSampleRate);
            size.prepare (newSampleRate);
            rotation.prepare (newSampleRate);
            translationX.prepare (newSampleRate); 
            translationY.prepare (newSampleRate);
            meanderanceScale.prepare (newSampleRate);
            meanderanceSpeed.prepare (newSampleRate);
            feedbackScalar.prepare (newSampleRate);
            feedbackTime.prepare (newSampleRate);
            feedbackCompression.prepare (newSampleRate);
            feedbackMix.prepare (newSampleRate);
            attack.prepare (newSampleRate);
            decay.prepare (newSampleRate);
            sustain.prepare (newSampleRate);
            release.prepare (newSampleRate);
        }
        tp::ChoiceParameter* currentTrajectory;
        SmoothedParameter mod_a, mod_b, mod_c, mod_d;
        SmoothedParameter size, rotation, translationX, translationY;
        SmoothedParameter meanderanceScale, meanderanceSpeed;
        SmoothedParameter feedbackScalar, feedbackTime, feedbackCompression, feedbackMix;
        juce::AudioParameterBool* envelopeSize;
        SmoothedParameter attack, decay, sustain, release;
    };
    VoiceParameters voiceParameters;
    PerlinVector perlinVector;
    float frequency = 440.0f;
    float amplitude = 1.0;
    double phase = 0.0;
    double phaseIncrement;
    double sampleRate = 48000.0;

    juce::Array<Point> feedbackBuffer;
    int feedbackWriteIndex = 0;
    int feedbackReadIndex;
    class History
    {
    public:
        History (int size = 4096) 
        {
            bufferSize = size * 3;
            buffer.allocate (bufferSize, false);
            clear();
            index = 0;
        }
    
        void feedNext (Point p, float o)
        {   
            buffer[index++] = p.x;
            buffer[index++] = p.y;
            buffer[index++] = o;
            index = index % bufferSize;
        }
        int size() { return bufferSize; }
        const float* getRawData() { return buffer.getData(); }
        void clear () 
        { 
            buffer.clear (bufferSize); 
            // std::memset (buffer.getData(), 1200, bufferSize * sizeof(float));
            // for (int i = 0; i < bufferSize; i++)
            //     buffer[i] = 400.0f;
        }
    private:
        // juce::Array<Position> buffer;
        juce::HeapBlock<float> buffer;
        int bufferSize;
        int index;
    }; 
    History history;

    void setFrequency (float newFrequency)
    {
        jassert (newFrequency > 0.0f);
        frequency = newFrequency;
        phaseIncrement = (frequency * juce::MathConstants<float>::twoPi) / sampleRate;
    }
    Point rotate (const Point p, const float theta)
    {
        Point newPoint ((p.x * std::cos (theta)) - (p.y * std::sin (theta)), 
                        (p.y * std::cos (theta)) + (p.x * std::sin (theta)));
        return newPoint;
    }
    Point scale (Point p, float scalar)
    {
        return p * (scalar);
    }
    Point translate (const Point p, float x, float y)
    {
        Point newPoint (p.x + x, p.y + y);
        return newPoint;
    }
    Point meander (const Point p, float scale)
    {
        auto meanderance = perlinVector.getNext() * scale;
        auto output = Point (p.x + meanderance.x, p.y + meanderance.y);
        return output;
    }
    Point feedback (Point input, float feedbackTime, float feedback, float mix, float threshold, float ratio)
    {
        feedbackReadIndex = feedbackWriteIndex - static_cast<int> ((feedbackTime * 0.001f) * sampleRate);
        if (feedbackReadIndex < 0) feedbackReadIndex += feedbackBuffer.size();
        auto scaledHistory = feedbackBuffer[feedbackReadIndex] * feedback;
        feedbackBuffer.set (feedbackWriteIndex, input + scaledHistory);
        feedbackWriteIndex = (feedbackWriteIndex + 1) % feedbackBuffer.size();
        auto outputPoint = input + (scaledHistory * mix);

        outputPoint = radialCompression (outputPoint, threshold, ratio);

        return outputPoint;
    }
    Point radialCompression (const Point p, float threshold, float ratio)
    {
        Point outputPoint = p;
        if (distance (p, Point (0.0f, 0.0f)) > threshold)
        {
            auto extraDistance = distance (p, normalize (p, threshold));
            auto adjustedDistance = extraDistance * (1.0f / ratio);
            outputPoint = normalize (p, threshold + adjustedDistance);
        }
        return outputPoint;
    }
    Point compressEdge (const Point p, float threshold = 1.0f, float ratio = 6.0f)
    {
        Point outputPoint = p;
        if (std::fabs (p.x) > threshold)
        {
            float adjustedExtension = (std::fabs (p.x) - threshold) * (1.0f / ratio);
            if (std::signbit (p.x)) // if negative
                outputPoint.x = -threshold - adjustedExtension;
            else 
                outputPoint.x = threshold + adjustedExtension;
        }
        if (std::fabs (p.y) > threshold)
        {
            float adjustedExtension = (std::fabs (p.y) - threshold) * (1.0f / ratio);
            if (std::signbit (p.y)) // if negative
                outputPoint.y = -threshold - adjustedExtension;
            else 
                outputPoint.y = threshold + adjustedExtension;
        }
        return outputPoint;
    }
    const ModSet getModSet()
     {
         return ModSet (voiceParameters.mod_a.getNext(), voiceParameters.mod_b.getNext(), 
                        voiceParameters.mod_c.getNext(), voiceParameters.mod_d.getNext());
    }
};

} // end namespace tp