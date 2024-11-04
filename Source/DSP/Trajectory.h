#pragma once 

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <MTS-ESP/Client/libMTSClient.h>
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

class Trajectory
{
public:
    Trajectory (Terrain& t,
                juce::ValueTree settingsBranch, 
                MTSClient& mtsc)
      : terrain (t),
        smoothFrequencyEnabled (settingsBranch, id::noteOnOrContinuous, nullptr),
        pitchBendRange (settingsBranch, id::pitchBendRange, nullptr),
        mtsClient (mtsc)
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
    void stopNote () { envelope.noteOff(); }
    void pitchWheelMoved (int newPitchWheelValue) { setPitchWheelIncrementScalar (newPitchWheelValue); }
    void controllerMoved () {}
    void renderNextBlock (juce::AudioBuffer<double>& ob, int ss, int nums) { juce::ignoreUnused (ob, ss, nums); }
    virtual void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, 
                                 int startSample, int numSamples) = 0;

    void setCurrentPlaybackSampleRate (double newRate) 
    {
        if (newRate > 0.0)
        {
            sampleRate = newRate;
            envelope.prepare (sampleRate);
            setFrequencyImmediate (frequency);
            perlinVector.setSampleRate (newRate);
        }
        // two second max delay
        feedbackBuffer.resize (static_cast<int> (sampleRate) * 2);
        feedbackBuffer.fill (Point(0.0f, 0.0f));
    }
    virtual void prepareToPlay (double newRate, int blockSize)
    {
        pitchWheelIncrementScalar.reset (newRate, 0.01);
        phaseIncrement.reset (blockSize);
        amplitude.reset (blockSize);
    }
    const float* getRawData() const { return history.getRawData(); }
    virtual void setState (juce::ValueTree settingsBranch)
    {
        pitchBendRange.referTo (settingsBranch, id::pitchBendRange, nullptr);
        smoothFrequencyEnabled.referTo (settingsBranch, id::noteOnOrContinuous, nullptr);
    }
    bool shouldClear() { return readyToClear; }
    void setFrequencyImmediate (float newFrequency)
    {
        jassert (newFrequency > 0.0f);
        frequency = newFrequency;
        phaseIncrement.setCurrentAndTargetValue ((frequency * juce::MathConstants<float>::twoPi) / sampleRate);
    }
    void setFrequencySmooth (float newFrequency)
    {
        jassert (newFrequency > 0.0f);
        frequency = newFrequency;
        phaseIncrement.setTargetValue ((frequency * juce::MathConstants<float>::twoPi) / sampleRate);
    }
    void setAmplitude (float newAmplitude) { amplitude.setTargetValue (newAmplitude); }
protected:
    Terrain& terrain;
    ADSR envelope;
    juce::Array<std::function<Point(float, ModSet)>> functions;

    PerlinVector perlinVector;
    float frequency = 440.0f;
    juce::SmoothedValue<float> amplitude;
    double phase = 0.0;
    int midiNote;
    juce::CachedValue<bool> smoothFrequencyEnabled;
    juce::SmoothedValue<double, juce::ValueSmoothingTypes::Multiplicative> phaseIncrement;
    juce::SmoothedValue<double, juce::ValueSmoothingTypes::Multiplicative> pitchWheelIncrementScalar {1.0};
    juce::CachedValue<float> pitchBendRange;
    double sampleRate = 48000.0;
    MTSClient& mtsClient;
    juce::Array<Point> feedbackBuffer;
    int feedbackWriteIndex = 0;
    int feedbackReadIndex;
    bool readyToClear = false;
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
        const float* getRawData() const { return buffer.getData(); }
        void clear () 
        { 
            buffer.clear (bufferSize); 
        }
    private:
        juce::HeapBlock<float> buffer;
        int bufferSize;
        int index;
    }; 
    History history;
    void setPitchWheelIncrementScalar (int pitchWheelPosition)
    {
        // linear mapping of 0 - 16383 to -1.0 - 1.0 will not work 
        // because the middle of the range is not 0.0f; thus we have
        // to branch each side of the 0
        float normalizedBend;
        if (pitchWheelPosition <= 8192)
            normalizedBend = (pitchWheelPosition - 8192) / 8192.0f;
        else
            normalizedBend = (pitchWheelPosition - 8191) / 8192.0f;
        
        float bendRangeSemitones = pitchBendRange.get(); // this will be a variable later; for now a constant bend range of a whole step
        float semitoneBend = normalizedBend * bendRangeSemitones;
        pitchWheelIncrementScalar.setTargetValue (std::pow (2.0, semitoneBend / 12.0));
    }
    void setPitchWheelIncrementScalar (double semitones)
    {
        pitchWheelIncrementScalar.setTargetValue (std::pow (2.0, semitones / 12.0f));
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Trajectory)
};
class StandardTrajectory : public Trajectory
{
public:
    StandardTrajectory (Terrain& t, 
                        Parameters& p, 
                        juce::ValueTree settingsBranch, 
                        MTSClient& mtsc)
      : Trajectory (t, settingsBranch, mtsc), 
        voiceParameters (p)
    {}
    void startNote (int midiNoteNumber,
                    float velocity,
                    int currentPitchWheelPosition) 
    {   
        setPitchWheelIncrementScalar (currentPitchWheelPosition);
        midiNote = midiNoteNumber;
        setFrequencyImmediate (static_cast<float> (MTS_NoteToFrequency (&mtsClient, 
                                                                        static_cast<char> (midiNote),
                                                                        -1)));
        readyToClear = false;
        if (MTS_ShouldFilterNote (&mtsClient, static_cast<char> (midiNote), -1)) 
            readyToClear = true; 

        amplitude.setCurrentAndTargetValue (velocity);
        envelope.noteOn();
        
        feedbackBuffer.fill (Point(0.0f, 0.0f));
    }
    void prepareToPlay (double newRate, int blockSize) override
    {
        Trajectory::prepareToPlay (newRate, blockSize);
        voiceParameters.resetSampleRate (newRate);
    }
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, 
                          int startSample, int numSamples) override
    {
        auto* o = outputBuffer.getWritePointer(0);
        if (smoothFrequencyEnabled.get())
            setFrequencySmooth (static_cast<float> (MTS_NoteToFrequency (&mtsClient, 
                                                                         static_cast<char> (midiNote), 
                                                                         -1)));
        for(int i = startSample; i < startSample + numSamples; i++)
        {
            if(!envelope.isActive()) break;
            tp::ADSR::Parameters p = {voiceParameters.attack.getNext(), 
                                      voiceParameters.decay.getNext(), 
                                      juce::Decibels::decibelsToGain (voiceParameters.sustain.getNext()), 
                                      voiceParameters.release.getNext()};
            envelope.setParameters (p);

            auto point = functions[*voiceParameters.currentTrajectory](static_cast<float> (phase), getModSet());
            
            float smoothAmplitude = amplitude.getNextValue();
            point = rotate (point, voiceParameters.rotation.getNext());
            point = scale (point, voiceParameters.size.getNext() * smoothAmplitude);
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

            float outputSample = terrain.sampleAt (point, i);
            history.feedNext (point, outputSample);
            o[i] += outputSample * static_cast<float> (envelope.calculateNext()) * smoothAmplitude;

            phase = std::fmod (phase + (phaseIncrement.getNextValue() * pitchWheelIncrementScalar.getNextValue()),
                               juce::MathConstants<double>::twoPi);

            if(!envelope.isActive())
            {
                history.clear();
                readyToClear = true;
            }
        }
    } 
private:
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
    const ModSet getModSet()
     {
         return ModSet (voiceParameters.mod_a.getNext(), voiceParameters.mod_b.getNext(), 
                        voiceParameters.mod_c.getNext(), voiceParameters.mod_d.getNext());
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandardTrajectory)
};

class MPETrajectory : public Trajectory
{
public:
    MPETrajectory (Terrain& t, 
                   Parameters& p, 
                   juce::ValueTree settingsBranch, 
                   MTSClient& mtsc,
                   juce::AudioProcessorValueTreeState& vts)
      : Trajectory (t, settingsBranch, mtsc),
        valueTreeState (vts),
        mpeRouting (settingsBranch.getChildWithName (id::MPE_ROUTING)),
        voiceParameters (p, vts, mpeRouting)
    {
        jassert (mpeRouting.getType() == id::MPE_ROUTING);
        juce::ignoreUnused (p);
    }
    void prepareToPlay (double newRate, int blockSize) override
    {
        Trajectory::prepareToPlay (newRate, blockSize);
        voiceParameters.resetSampleRate (newRate);
    }
    void startNote (int midiNoteNumber,
                    float velocity, 
                    float frequencyHz, 
                    float pressure, 
                    float timbre) 
    {   
        midiNote = midiNoteNumber;
        setFrequencyImmediate (static_cast<float> (MTS_NoteToFrequency (&mtsClient, 
                                                                        (MTS_FrequencyToNote (&mtsClient, 
                                                                                              static_cast<double> (frequencyHz), 
                                                                                              -1)),
                                                                        -1)));
        readyToClear = false;
        if (MTS_ShouldFilterNote (&mtsClient, static_cast<char> (midiNote), -1)) 
            readyToClear = true; 

        amplitude.setCurrentAndTargetValue (velocity);
        envelope.noteOn();
        voiceParameters.noteOn (pressure, timbre);
        feedbackBuffer.fill (Point(0.0f, 0.0f));
    }
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, 
                          int startSample, int numSamples) override
    {
        auto* o = outputBuffer.getWritePointer(0);
        if (smoothFrequencyEnabled.get())
            setFrequencySmooth (static_cast<float> (MTS_NoteToFrequency (&mtsClient, 
                                                                         static_cast<char> (midiNote), 
                                                                         -1)));
        for(int i = startSample; i < startSample + numSamples; i++)
        {
            if(!envelope.isActive()) break;
            tp::ADSR::Parameters p = {voiceParameters.attack.getNext(), 
                                      voiceParameters.decay.getNext(), 
                                      juce::Decibels::decibelsToGain (voiceParameters.sustain.getNext()), 
                                      voiceParameters.release.getNext()};
            envelope.setParameters (p);

            auto point = functions[*voiceParameters.currentTrajectory](static_cast<float> (phase), getModSet());
            
            float smoothAmplitude = amplitude.getNextValue();
            point = rotate (point, voiceParameters.rotation.getNext());
            point = scale (point, voiceParameters.size.getNext());
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

            float outputSample = terrain.sampleAt (point, i);
            history.feedNext (point, outputSample);
            o[i] += outputSample * static_cast<float> (envelope.calculateNext()) * smoothAmplitude;

            phase = std::fmod (phase + (phaseIncrement.getNextValue() * pitchWheelIncrementScalar.getNextValue()),
                               juce::MathConstants<double>::twoPi);

            if(!envelope.isActive())
            {
                history.clear();
                readyToClear = true;
            }
        }
    } 
    void setPressure (float newPressure) { voiceParameters.setPressure (newPressure); }
    void setTimbre (float newTimbre) { voiceParameters.setTimbre (newTimbre); }
    void setState (juce::ValueTree settingsBranch) override
    {
        Trajectory::setState (settingsBranch);
        mpeRouting = settingsBranch.getChildWithName (id::MPE_ROUTING);
        voiceParameters.setState (mpeRouting);
    }
private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::ValueTree mpeRouting;
    struct VoiceParameters
    {
        VoiceParameters (Parameters& p, 
                         juce::AudioProcessorValueTreeState& valueTreeState, 
                         juce::ValueTree mpeRouting)
          : currentTrajectory (p.currentTrajectory),
            mod_a (p.trajectoryModA, valueTreeState, mpeRouting),
            mod_b (p.trajectoryModB, valueTreeState, mpeRouting),
            mod_c (p.trajectoryModC, valueTreeState, mpeRouting),
            mod_d (p.trajectoryModD, valueTreeState, mpeRouting), 
            size (p.trajectorySize, valueTreeState, mpeRouting), 
            rotation (p.trajectoryRotation, valueTreeState, mpeRouting), 
            translationX (p.trajectoryTranslationX, valueTreeState, mpeRouting), 
            translationY (p.trajectoryTranslationY, valueTreeState, mpeRouting), 
            meanderanceScale (p.meanderanceScale, valueTreeState, mpeRouting),
            meanderanceSpeed (p.meanderanceSpeed, valueTreeState, mpeRouting),
            feedbackScalar (p.feedbackScalar, valueTreeState, mpeRouting), 
            feedbackTime (p.feedbackTime, valueTreeState, mpeRouting), 
            feedbackCompression (p.feedbackCompression, valueTreeState, mpeRouting),
            feedbackMix (p.feedbackMix, valueTreeState, mpeRouting), 
            envelopeSize (p.envelopeSize),
            attack (p.attack, valueTreeState, mpeRouting), 
            decay (p.decay, valueTreeState, mpeRouting), 
            sustain (p.sustain, valueTreeState, mpeRouting), 
            release (p.release, valueTreeState, mpeRouting)
        {}
        void noteOn (float timbre, float pressure)
        {
            mod_a.noteOn(timbre, pressure);
            mod_b.noteOn(timbre, pressure);
            mod_c.noteOn(timbre, pressure);
            mod_d.noteOn(timbre, pressure);
            size.noteOn(timbre, pressure);
            rotation.noteOn(timbre, pressure);
            translationX.noteOn(timbre, pressure);
            translationY.noteOn(timbre, pressure);
            meanderanceScale.noteOn(timbre, pressure);
            meanderanceSpeed.noteOn(timbre, pressure);
            feedbackScalar.noteOn(timbre, pressure);
            feedbackTime.noteOn(timbre, pressure);
            feedbackCompression.noteOn(timbre, pressure);
            feedbackMix.noteOn(timbre, pressure);
            attack.noteOn(timbre, pressure);
            decay.noteOn(timbre, pressure);
            sustain.noteOn(timbre, pressure);
            release.noteOn(timbre, pressure);
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
        void setTimbre (float newTimbre)
        {
            mod_a.setTimbre (newTimbre);
            mod_b.setTimbre (newTimbre);
            mod_c.setTimbre (newTimbre);
            mod_d.setTimbre (newTimbre);
            size.setTimbre (newTimbre);
            rotation.setTimbre (newTimbre);
            translationX.setTimbre (newTimbre); 
            translationY.setTimbre (newTimbre);
            meanderanceScale.setTimbre (newTimbre);
            meanderanceSpeed.setTimbre (newTimbre);
            feedbackScalar.setTimbre (newTimbre);
            feedbackTime.setTimbre (newTimbre);
            feedbackCompression.setTimbre (newTimbre);
            feedbackMix.setTimbre (newTimbre);
            attack.setTimbre (newTimbre);
            decay.setTimbre (newTimbre);
            sustain.setTimbre (newTimbre);
            release.setTimbre (newTimbre);
        }
        void setPressure (float newPressure)
        {
            mod_a.setPressure (newPressure);
            mod_b.setPressure (newPressure);
            mod_c.setPressure (newPressure);
            mod_d.setPressure (newPressure);
            size.setPressure (newPressure);
            rotation.setPressure (newPressure);
            translationX.setPressure (newPressure); 
            translationY.setPressure (newPressure);
            meanderanceScale.setPressure (newPressure);
            meanderanceSpeed.setPressure (newPressure);
            feedbackScalar.setPressure (newPressure);
            feedbackTime.setPressure (newPressure);
            feedbackCompression.setPressure (newPressure);
            feedbackMix.setPressure (newPressure);
            attack.setPressure (newPressure);
            decay.setPressure (newPressure);
            sustain.setPressure (newPressure);
            release.setPressure (newPressure);
        }
        void setState (juce::ValueTree mpeRoutingBranch)
        {
            mod_a.setState (mpeRoutingBranch);
            mod_b.setState (mpeRoutingBranch);
            mod_c.setState (mpeRoutingBranch);
            mod_d.setState (mpeRoutingBranch);
            size.setState (mpeRoutingBranch);
            rotation.setState (mpeRoutingBranch);
            translationX.setState (mpeRoutingBranch); 
            translationY.setState (mpeRoutingBranch);
            meanderanceScale.setState (mpeRoutingBranch);
            meanderanceSpeed.setState (mpeRoutingBranch);
            feedbackScalar.setState (mpeRoutingBranch);
            feedbackTime.setState (mpeRoutingBranch);
            feedbackCompression.setState (mpeRoutingBranch);
            feedbackMix.setState (mpeRoutingBranch);
            attack.setState (mpeRoutingBranch);
            decay.setState (mpeRoutingBranch);
            sustain.setState (mpeRoutingBranch);
            release.setState (mpeRoutingBranch);           
        }
        tp::ChoiceParameter* currentTrajectory;
        MPESmoothedParameter mod_a, mod_b, mod_c, mod_d;
        MPESmoothedParameter size, rotation, translationX, translationY;
        MPESmoothedParameter meanderanceScale, meanderanceSpeed;
        MPESmoothedParameter feedbackScalar, feedbackTime, feedbackCompression, feedbackMix;
        juce::AudioParameterBool* envelopeSize;
        MPESmoothedParameter attack, decay, sustain, release;
    };
    VoiceParameters voiceParameters;
    const ModSet getModSet()
     {
         return ModSet (voiceParameters.mod_a.getNext(), voiceParameters.mod_b.getNext(), 
                        voiceParameters.mod_c.getNext(), voiceParameters.mod_d.getNext());
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPETrajectory)
};
} // end namespace tp