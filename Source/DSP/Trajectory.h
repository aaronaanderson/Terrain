#pragma once 

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>

#include <MTS-ESP/Client/libMTSClient.h>

#include <morphlib/BandPassFilter.h>

#include "../Parameters.h"
#include "DataTypes.h"
#include "ADSR.h"
#include "Terrain.h"
#include "RadialCompressor.h"
#include "Meanderer.h"

#include "MPEVoiceData.h"
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
    virtual void stopNote () { envelope.noteOff(); }
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
            meanderer.prepare (newRate);
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
    virtual void setFrequencyImmediate (float newFrequency)
    {
        jassert (newFrequency > 0.0f);
        frequency = newFrequency;
        phaseIncrement.setCurrentAndTargetValue ((frequency * juce::MathConstants<float>::twoPi) / sampleRate);
    }
    virtual void setFrequencySmooth (float newFrequency)
    {
        jassert (newFrequency > 0.0f);
        frequency = newFrequency;
        phaseIncrement.setTargetValue ((frequency * juce::MathConstants<float>::twoPi) / sampleRate);
    }
protected:
    Terrain& terrain;
    ADSR envelope;
    juce::Array<std::function<Point(float, ModSet)>> functions;
    Meanderer meanderer;
    float frequency = 440.0f;
    juce::SmoothedValue<float> amplitude;
    double phase = 0.0;
    int midiNote = 0;
    juce::CachedValue<bool> smoothFrequencyEnabled;
    juce::SmoothedValue<double, juce::ValueSmoothingTypes::Multiplicative> phaseIncrement;
    juce::SmoothedValue<double, juce::ValueSmoothingTypes::Multiplicative> pitchWheelIncrementScalar {1.0};
    juce::CachedValue<float> pitchBendRange;
    double sampleRate = 48000.0;
    MTSClient& mtsClient;
    juce::Array<Point> feedbackBuffer;
    int feedbackWriteIndex = 0;
    int feedbackReadIndex = 0;
    bool readyToClear = false;
    class History
    {
    public:
        explicit History (int size = 4096)
        {
            bufferSize = size * 3;
            buffer.allocate (bufferSize, false);
            clear();
            index = 0;
        }
    
        void feedNext (Point p, float o)
        {   
            buffer[index] = p.x;
            buffer[(index + 1) % bufferSize] = p.y;
            buffer[(index + 2) % bufferSize] = o;
            index = (index + 3) % bufferSize;
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
    void setAmplitude (float newAmplitude) { amplitude.setTargetValue (newAmplitude); }
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
        
        float bendRangeSemitones = pitchBendRange.get();
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

    Point feedback (Point input, float feedbackTime, float feedback, float mix)
    {
        auto delayInSamples = static_cast<int>((feedbackTime * 0.001f) * sampleRate);
        delayInSamples = std::min(delayInSamples, feedbackBuffer.size() - 1);
        feedbackReadIndex = feedbackWriteIndex - delayInSamples;
        if (feedbackReadIndex < 0) feedbackReadIndex += feedbackBuffer.size();
        auto scaledHistory = feedbackBuffer[feedbackReadIndex] * feedback;
        feedbackBuffer.set (feedbackWriteIndex, input + scaledHistory);
        feedbackWriteIndex = (feedbackWriteIndex + 1) % feedbackBuffer.size();
        auto outputPoint = input + (scaledHistory * mix);

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

class MPETrajectory : public Trajectory
{
public:
    MPETrajectory (Terrain& t, 
                   Parameters& p, 
                   juce::ValueTree settingsBranch, 
                   MTSClient& mtsc,
                   juce::AudioProcessorValueTreeState& vts, 
                   MPEVoiceData& vd)
      : Trajectory (t, settingsBranch, mtsc),
        mpeRouting (settingsBranch.getChildWithName (id::MPE_ROUTING)),
        voiceData (vd),
        voiceParameters (p, vts, mpeRouting)
    {
        jassert (mpeRouting.getType() == id::MPE_ROUTING);
        juce::ignoreUnused (p);
    }
    void prepareToPlay (double newRate, int blockSize) override
    {
        Trajectory::prepareToPlay (newRate, blockSize);
        voiceParameters.resetSampleRate (newRate);
        renderBuffer.setSize (1, blockSize, false, false, true);
        renderBuffer.clear (0, blockSize);

        processSpec.maximumBlockSize = static_cast<juce::uint32> (blockSize);
        processSpec.numChannels = 1;
        processSpec.sampleRate = newRate;
        ladderFilter.prepare (processSpec);
        ladderFilter.setCutoffFrequencyHz (400.0f);
        ladderFilter.setResonance (0.7f);
        smoothRMS.reset (4);

        radialCompressor.prepare (newRate);
        bandPassFilter.prepare (newRate, blockSize);
    }
    void startNote (int midiNoteNumber,
                    float adjustedFrequency,
                    float velocity, 
                    float frequencyHz,
                    float pressure, 
                    float timbre, 
                    int channel) 
    {   
        juce::ignoreUnused (frequencyHz);
        midiNote = midiNoteNumber;
        midiChannel = channel;
        setPitchWheelIncrementScalar (0.0);

        setFrequencyImmediate (static_cast<float> (adjustedFrequency));
        readyToClear = false;
        if (MTS_ShouldFilterNote (&mtsClient, static_cast<char> (midiNoteNumber), -1)) 
            readyToClear = true; 
        
        float scaledVelocity = juce::jmap (velocity, 0.0f, 1.0f, juce::Decibels::decibelsToGain (-voiceParameters.sensitivity.getNext()), 1.0f);
        amplitude.setCurrentAndTargetValue (scaledVelocity);
        envelope.noteOn();
        voiceParameters.noteOn (pressure, timbre);
        feedbackBuffer.fill (Point(0.0f, 0.0f));
        rmsUpdater = std::make_unique<RMSUpdater> (voiceData, midiChannel, currentRMS);
        rmsUpdater->startTimerHz (24);

        radialCompressor.reset();
        bandPassFilter.noteOn();
    }
    void stopNote() override
    {
        if (rmsUpdater.get() != nullptr)
            rmsUpdater->stopTimer();
        Trajectory::stopNote();
    }

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, 
                          int startSample, int numSamples) override
    {
        auto* o = renderBuffer.getWritePointer(0);
        //if (smoothFrequencyEnabled.get())
        //{
        //    setFrequencySmooth (static_cast<float> (frequency));
        //    voiceParameters.pitch.skip(numSamples);
        //}
        for(int i = startSample; i < startSample + numSamples; i++)
        {
            juce::ignoreUnused (voiceParameters.pitch.getNext()); // hack, hopefully I'll get back to removing this!
            if(!envelope.isActive()) break;
            tp::ADSR::Parameters p = {voiceParameters.attack.getNext(), 
                                      voiceParameters.decay.getNext(), 
                                      voiceParameters.sustain.getNext(), 
                                      voiceParameters.release.getNext()};
            envelope.setParameters (p);

            auto point = functions[*voiceParameters.currentTrajectory](static_cast<float> (phase), getModSet());
            
            point = rotate (point, voiceParameters.rotation.getNext());
            point = scale (point, voiceParameters.size.getNext());
            if (*voiceParameters.envelopeSize)
                point = scale (point, static_cast<float> (envelope.getCurrentValue()));
            float adjustedFrequency = frequency * std::pow (2.0f, voiceParameters.feedbackTime.getNext());
            point = feedback (point, 
                              /*voiceParameters.feedbackTime.getNext()*/ ( 1000.0f / adjustedFrequency ), 
                              voiceParameters.feedbackScalar.getNext(), 
                              voiceParameters.feedbackMix.getNext());

            radialCompressor.setThreshold( voiceParameters.radialCompressorThreshold.getNext());
            radialCompressor.setRatio( voiceParameters.radialCompressorRatio.getNext());
            radialCompressor.setResponsiveness( juce::jmap (voiceParameters.radialCompressorResponsiveness.getNext(), 0.0f, 1.0f, 200.0f, 1.0f) );
            radialCompressor.setKnee (0.0f);
            juce::Point pointCast { point.x, point.y };
            auto jucePoint = radialCompressor.processPoint (pointCast);
            point = { jucePoint.getX(), jucePoint.getY() };

            point = translate (point, 
                               voiceParameters.translationX.getNext(), 
                               voiceParameters.translationY.getNext());

            meanderer.setSpeed (voiceParameters.meanderanceSpeed.getNext());
            meanderer.setScale (voiceParameters.meanderanceScale.getNext());
            meanderer.setCaffiene (voiceParameters.meanderanceCaffiene.getNext());
            auto jPoint = meanderer.process ({point.x, point.y});
            point = {jPoint.x, jPoint.y};

            point = compressEdge (point);

            float outputSample = terrain.sampleAt (point, i);
            history.feedNext (point, outputSample);
            float smoothAmplitude = amplitude.getNextValue(); // set from scaled velocity
            float env = static_cast<float> (envelope.calculateNext());
            setRMS (env);
            o[i] = outputSample * env * smoothAmplitude * voiceParameters.amplitude.getNext();

            double pitchScalar = getPitchScalar (voiceParameters.pitch.getNext() + voiceParameters.cents.getNext() * 0.01f);
            pitchScalar *= pitchWheelIncrementScalar.getNextValue();
            phase = std::fmod (phase + (phaseIncrement.getNextValue() * pitchScalar), juce::MathConstants<double>::twoPi);

            if(!envelope.isActive())
            {
                history.clear();
                readyToClear = true;
            }
        }
        
        scratchBuffer.setSize (1, numSamples, false, false, true);
        for (int i = 0; i < numSamples; i++)
            scratchBuffer.getWritePointer (0)[i] = renderBuffer.getReadPointer (0)[startSample + i];
        
        /*
        //===== Moog Filter =====
        */
        if (*voiceParameters.filterBypass)
        {
            auto outputBlock = juce::dsp::AudioBlock<float> (scratchBuffer);
            juce::dsp::ProcessContextReplacing<float> context (outputBlock);
            
            float freqScalar = static_cast<float> (std::pow (2, voiceParameters.filterFrequency.getNext()));
            ladderFilter.setCutoffFrequencyHz (frequency * freqScalar);
            ladderFilter.setResonance (voiceParameters.filterResonance.getNext());
            ladderFilter.process (context);
        }
        /*
        //===== BAND PASS FILTER =====
        */
        float cf = voiceParameters.bandPassCenterFreq.getNext();
        float bw = voiceParameters.bandPassBandwidth.getNext();
        float low  = juce::jlimit (20.0f, 20000.0f, cf * std::pow (2.0f, -bw));
        float high = juce::jlimit (20.0f, 20000.0f, cf * std::pow (2.0f,  bw));

        bandPassFilter.setLowFrequency (low);
        bandPassFilter.setHighFrequency (high);
        bandPassFilter.process(scratchBuffer);

        // copy from scratch buffer, adding to incoming content
        for (int i = 0; i < numSamples; i++)
            outputBuffer.getWritePointer (0)[i + startSample] += scratchBuffer.getReadPointer (0)[i];

        /*
        //===== PANNING =====
        */
        jassert (outputBuffer.getNumChannels() == 2);
        auto* l = outputBuffer.getWritePointer (0);
        auto* r = outputBuffer.getWritePointer (1);
        for (int i = 0; i < numSamples; i++) 
        {
            float signal = outputBuffer.getSample (0, startSample + i);
            float panPosition = voiceParameters.pan.getNext();
            auto [left, right] = equalPowerPan (panPosition, signal);
            l[startSample + i] += left;
            r[startSample + i] += right;
        }
    } 
    void setPressure (float newPressure) { voiceParameters.setPressure (newPressure); }
    void setTimbre (float newTimbre) { voiceParameters.setTimbre (newTimbre); }
    void setPressureSmoothing (float ms) { voiceParameters.setPressureSmoothing (ms); }
    void setTimbreSmoothing (float ms) { voiceParameters.setTimbreSmoothing (ms); }

    void setState (juce::ValueTree settingsBranch) override
    {
        Trajectory::setState (settingsBranch);
        mpeRouting = settingsBranch.getChildWithName (id::MPE_ROUTING);
        voiceParameters.setState (mpeRouting);
    }
    void setRelease() {envelope.setPhase (ADSR::Phase::RELEASE); }
    void allocate (int maximumSamplesPerBlock) 
    {
        scratchBuffer.setSize (1, maximumSamplesPerBlock);
        scratchBuffer.clear (0, maximumSamplesPerBlock);
        renderBuffer.setSize (1, maximumSamplesPerBlock); 
        renderBuffer.clear (0, maximumSamplesPerBlock);
    }
    float getRMS() { return currentRMS; }

    void setFrequencyImmediate (float newFrequency) override
    {
        jassert (newFrequency > 0.0f);
        frequency = newFrequency;
        phaseIncrement.setCurrentAndTargetValue ((frequency * juce::MathConstants<float>::twoPi) / sampleRate);
    }
    void setFrequencySmooth (float newFrequency) override
    {
        jassert (newFrequency > 0.0f);
        frequency = newFrequency;
        DBG(frequency);
        phaseIncrement.setTargetValue ((frequency * juce::MathConstants<float>::twoPi) / sampleRate);
    }
private:
    juce::ValueTree mpeRouting;
    juce::AudioBuffer<float> renderBuffer;
    juce::AudioBuffer<float> scratchBuffer;

    RadialCompressor radialCompressor;

    juce::dsp::LadderFilter<float> ladderFilter;
    morph::BandPassFilter bandPassFilter;
    juce::dsp::ProcessSpec processSpec;
    
    juce::SmoothedValue<float> smoothRMS {0.0f};

    MPEVoiceData& voiceData;
    int midiChannel;
    struct VoiceParameters
    {
        VoiceParameters (Parameters& p, 
                         juce::AudioProcessorValueTreeState& vts, 
                         juce::ValueTree MPERouting)
          : currentTrajectory (p.currentTrajectory),
            mod_a (p.trajectoryModA, vts, MPERouting),
            mod_b (p.trajectoryModB, vts, MPERouting),
            mod_c (p.trajectoryModC, vts, MPERouting),
            mod_d (p.trajectoryModD, vts, MPERouting), 
            amplitude (p.trajectoryAmplitude, vts, MPERouting),
            pan (p.trajectoryPan, vts, MPERouting),
            pitch (p.trajectoryPitch, vts, MPERouting),
            cents (p.trajectoryCents, vts, MPERouting),
            size (p.trajectorySize, vts, MPERouting), 
            rotation (p.trajectoryRotation, vts, MPERouting), 
            translationX (p.trajectoryTranslationX, vts, MPERouting), 
            translationY (p.trajectoryTranslationY, vts, MPERouting), 
            meanderanceScale (p.meanderanceScale, vts, MPERouting),
            meanderanceSpeed (p.meanderanceSpeed, vts, MPERouting),
            meanderanceCaffiene (p.meanderanceCaffiene, vts, MPERouting),
            feedbackScalar (p.feedbackScalar, vts, MPERouting), 
            feedbackTime (p.combFrequency, vts, MPERouting), 
            feedbackMix (p.feedbackMix, vts, MPERouting), 
            envelopeSize (p.envelopeSize),
            attack (p.attack, vts, MPERouting), 
            decay (p.decay, vts, MPERouting), 
            sustain (p.sustain, vts, MPERouting), 
            release (p.release, vts, MPERouting), 
            sensitivity (p.sensitivity, vts, MPERouting),
            filterFrequency (p.perVoiceFilterFrequency, vts, MPERouting), 
            filterResonance (p.perVoiceFilterResonance, vts, MPERouting),
            filterBypass (p.perVoiceFilterOnOff),
            radialCompressorThreshold (p.radialCompressorThreshold, vts, MPERouting),
            radialCompressorRatio (p.radialCompressorRatio, vts, MPERouting),
            radialCompressorResponsiveness (p.radialCompressorResponsiveness, vts, MPERouting),
            bandPassCenterFreq (p.voiceBandPassCenterFreq, vts, MPERouting),
            bandPassBandwidth (p.voiceBandPassBandwidth, vts, MPERouting)
        {
            sensitivity.setControlSmoothing (0.0);
            filterFrequency.setControlSmoothing (0.0);
            filterResonance.setControlSmoothing (0.0);
            bandPassCenterFreq.setControlSmoothing (0.0);
            bandPassBandwidth.setControlSmoothing (0.0);
        }

        /*
            To add parameters, add it as a member, construct it in the list, add it to parameters, and
            increment the list size. 
            If it's special in some way, for example it doesn't want smoothing, place it in the appropriate
            exception group.
        */

        tp::ChoiceParameter* currentTrajectory;
        MPESmoothedParameter mod_a, mod_b, mod_c, mod_d;
        MPESmoothedParameter amplitude, pan, pitch, cents, size, rotation, translationX, translationY;
        MPESmoothedParameter meanderanceScale, meanderanceSpeed, meanderanceCaffiene;
        MPESmoothedParameter feedbackScalar, feedbackTime, feedbackMix;
        MPESmoothedParameter radialCompressorThreshold, radialCompressorRatio, radialCompressorResponsiveness;
        juce::AudioParameterBool* envelopeSize;
        MPESmoothedParameter attack, decay, sustain, release, sensitivity;
        MPESmoothedParameter filterFrequency, filterResonance;
        juce::AudioParameterBool* filterBypass;
        MPESmoothedParameter bandPassCenterFreq, bandPassBandwidth;

        std::array<MPESmoothedParameter*, 30> parameters 
        {
            &mod_a,&mod_b,&mod_c,&mod_d,
            &amplitude, &pan, &pitch, &cents, &size,&rotation,&translationX,&translationY,
            &meanderanceScale,&meanderanceSpeed, &meanderanceCaffiene,
            &feedbackScalar,&feedbackTime,&feedbackMix,
            &attack,&decay,&sustain,&release,&sensitivity,
            &filterFrequency,&filterResonance,
            &radialCompressorThreshold,&radialCompressorRatio,&radialCompressorResponsiveness,
            &bandPassCenterFreq, &bandPassCenterFreq
        };
        // Exception groups
        std::array<MPESmoothedParameter*, 5> noPressureSmooth { &sensitivity ,&filterFrequency, &filterResonance, &bandPassBandwidth, &bandPassCenterFreq };
        std::array<MPESmoothedParameter*, 5> noTimbreSmooth   { &sensitivity, &filterFrequency,&filterResonance, &bandPassBandwidth, &bandPassCenterFreq };

        template <class F> void forAll(F&& f) { for (auto* p : parameters) f(*p); }

        void resetSampleRate (double sr)       { forAll([&](auto& x){ x.prepare(sr); }); }
        void noteOn (float t, float p)         { forAll([&](auto& x){ x.noteOn(t, p); }); }
        void setTimbre (float v)               { forAll([&](auto& x){ x.setTimbre(v); }); }
        void setPressure (float v)             { forAll([&](auto& x){ x.setPressure(v); }); }
        void setState (juce::ValueTree vt)     { forAll([&](auto& x){ x.setState(vt); }); }
        void setPressureSmoothing (float ms)   { 
            forAll([&](auto& x){ x.setPressureSmoothing(ms); });
            for (auto* p : noPressureSmooth) p->setPressureSmoothing(0.0f); }
        void setTimbreSmoothing (float ms)     { 
            forAll([&](auto& x){ x.setTimbreSmoothing(ms); });
            for (auto* p : noTimbreSmooth)   p->setTimbreSmoothing(0.0f); }
    };
    VoiceParameters voiceParameters;
    const ModSet getModSet()
     {
         return ModSet (voiceParameters.mod_a.getNext(), voiceParameters.mod_b.getNext(), 
                        voiceParameters.mod_c.getNext(), voiceParameters.mod_d.getNext());
    }
    std::atomic<float> currentRMS{0.0f};
    void setRMS (float newRMS) { currentRMS.store (newRMS); }
    class RMSUpdater : public juce::Timer
    {
    public:
        RMSUpdater (MPEVoiceData& vd, int channel, std::atomic<float>& rms)
          : voiceData (vd), midiChannel (channel), currentRMS (rms)
        {
            smoothRMS.reset (20);
        }
        void timerCallback() override
        {
            float adjustedRMS = juce::jlimit(0.0f, 1.0f, 
                juce::jmap(smoothRMS.getNextValue(), 0.0f, 0.5f, 0.0f, 2.0f));
            smoothRMS.setTargetValue (currentRMS.load());
            voiceData.setRMSAT (adjustedRMS, midiChannel - 2);
        }
    private:
        MPEVoiceData& voiceData;
        int midiChannel;
        std::atomic<float>& currentRMS;
        juce::SmoothedValue<float> smoothRMS {0.0f};
    };
    std::unique_ptr<RMSUpdater> rmsUpdater;

    const double ONE_TWELFTH = 1.0 / 12.0;
    inline double getPitchScalar(float pitch) { return std::pow(2.0, pitch * ONE_TWELFTH); }
    inline std::pair<float, float> equalPowerPan (const float position, const float input) 
    {
        float pan = juce::jlimit (-1.0f, 1.0f, position);
        float theta = 0.25f * juce::MathConstants<float>::twoPi * (pan + 1.0f);
        return { input * std::cos (theta), 
                 input * std::sin (theta) };
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPETrajectory)
};
} // end namespace tp