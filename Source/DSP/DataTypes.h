#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <PerlinNoise/PerlinNoise.hpp>

namespace tp {
struct Point 
{  
    Point (float xp = 0.0f, float yp = 0.0f) : x(xp), y(yp) {}
    float x; float y; 

    Point operator+(const Point& other) { return Point(this->x + other.x, this->y + other.y); }
    Point operator*(float scalar) { return Point(this->x * scalar, this->y * scalar); }
};
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
    Point getNext()
    {
        incrementSampleIndex();
        return Point (smoothX.getNextValue(), smoothY.getNextValue());
    }
    void setSpeed (double newSpeed) // 0 - 1 expected (arbitrary decision)
    {
        phaseIncrement = newSpeed * inverseSampleRate * 1500.0;
    }
    void setSampleRate (double newSampleRate)
    {
        juce::ignoreUnused (newSampleRate);
        // sampleInterval = static_cast<int> (newSampleRate * (48000.0 / 512.0));
        inverseSampleRate = 1.0 / newSampleRate;
    }
    private:
    siv::BasicPerlinNoise<float> noiseX, noiseY;
    juce::SmoothedValue<float> smoothX, smoothY;
    double inverseSampleRate;
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
struct ModSet
{
    ModSet (float ia = 0.0f, float ib = 0.0f, float ic = 0.0f, float id = 0.0f) : a(ia), b(ib), c(ic), d(id) {}
    float a, b, c, d;
};
struct SmoothedParameter : private juce::AudioProcessorParameter::Listener
{
public:
    SmoothedParameter (juce::RangedAudioParameter* p)
      : rangedParameter (p), 
        smoothedValue (p->convertFrom0to1 (p->getValue()))
    {
        rangedParameter->addListener (this);
    }
    ~SmoothedParameter() override { rangedParameter->removeListener (this); }
    void noteOn() { smoothedValue.setCurrentAndTargetValue (rangedParameter->convertFrom0to1 (rangedParameter->getValue())); }
    float getNext() { return smoothedValue.getNextValue(); }
    float getCurrent() { return smoothedValue.getCurrentValue(); }
    void prepare (double sr) 
    { 
        sampleRate = sr;
        smoothedValue.reset (sampleRate, timeMS * 0.001f); 
    }
    void setTimeMS (float time) 
    {
        timeMS = time;
        smoothedValue.reset (sampleRate, timeMS);
        smoothedValue.setCurrentAndTargetValue (rangedParameter->convertFrom0to1 (rangedParameter->getValue()));
    }
private:
    juce::RangedAudioParameter* rangedParameter;
    juce::SmoothedValue<float> smoothedValue;
    double sampleRate = 48000.0;
    float timeMS = 20.0f;
    void parameterValueChanged (int parameterIndex, float newValue) override
    {
        juce::ignoreUnused (parameterIndex);
        smoothedValue.setTargetValue (rangedParameter->convertFrom0to1 (newValue));
    }
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override { juce::ignoreUnused (parameterIndex, gestureIsStarting); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SmoothedParameter)
};
class BufferedSmoothParameter
{
public:
    BufferedSmoothParameter (juce::RangedAudioParameter* p)
      : smoothedParameter (p)
    {}
    void prepareToPlay (double sr, int blockSize)
    {
        smoothedParameter.prepare (sr);
        // buffer.resize (blockSize);
        buffer.setSize (1, blockSize, false, false, true);
    }
    // call once per audio block
    void updateBuffer()
    {
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            auto* b = buffer.getWritePointer (0);
            b[i] = smoothedParameter.getNext();
        }
    }
    float getAt (int bufferIndex) { return buffer.getReadPointer (0)[bufferIndex]; }
    void allocate (int numSamples) { buffer.setSize (1, numSamples); }
private:
    SmoothedParameter smoothedParameter;
    juce::AudioBuffer<float> buffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferedSmoothParameter)
};

struct MPESmoothedParameter : private juce::AudioProcessorParameter::Listener
{
public:
    MPESmoothedParameter (juce::RangedAudioParameter* p, 
                          juce::AudioProcessorValueTreeState& vts, 
                          juce::ValueTree mpeRoutingBranch)
      : rangedParameter (p), 
        valueTreeState (vts),
        mpeRouting (mpeRoutingBranch),
        smoothedValue (p->convertFrom0to1 (p->getValue()))
    {
        jassert (mpeRouting.getType() == id::MPE_ROUTING);
        rangedParameter->addListener (this);
        checkAssignment();
    }
    ~MPESmoothedParameter() override { rangedParameter->removeListener (this); }
    void noteOn (float mpePressure, float mpeTimbre) 
    { 
        checkAssignment();
        switch (assignment)
        {
            case Assignment::None:
                smoothedValue.setCurrentAndTargetValue (rangedParameter->convertFrom0to1 (rangedParameter->getValue())); 
            break;
            case Assignment::Pressure:
                {
                    jassert (outputChannel.isValid());
                    float min = outputChannel.getProperty (id::lowerBound);
                    float max = outputChannel.getProperty (id::upperBound);
                    bool inv = outputChannel.getProperty (id::invertRange);
                    auto value = inv ? juce::jmap (mpePressure, max, min) : 
                                       juce::jmap (mpePressure, min, max);
                    smoothedPressure.setCurrentAndTargetValue (rangedParameter->convertFrom0to1 (value));
                }; 
            break;
            case Assignment::Timbre:
                {
                    jassert (outputChannel.isValid());
                    float min = outputChannel.getProperty (id::lowerBound);
                    float max = outputChannel.getProperty (id::upperBound);
                    bool inv = outputChannel.getProperty (id::invertRange);
                    auto value = inv ? juce::jmap (mpeTimbre, max, min) : 
                                       juce::jmap (mpeTimbre, min, max);
                    smoothedTimbre.setCurrentAndTargetValue (rangedParameter->convertFrom0to1 (value));
                }
            break;
            default:
                jassertfalse;
        }
    }
    void setPressure (float p) 
    { 
        pressure = p;
        if (assignment != Assignment::Pressure) return;
        setPressureInternal (p); 
    }
    void setTimbre (float t)
    {
        timbre = t;
        if (assignment != Assignment::Timbre) return;
        setTimbreInternal (t); 
    }
    float getNext() 
    { 
        switch (assignment)
        {
            case Assignment::None: return smoothedValue.getNextValue(); 
            case Assignment::Pressure: return smoothedPressure.getNextValue();
            case Assignment::Timbre: return smoothedTimbre.getNextValue();
            default: jassertfalse;
        }
        return 0.0f;
    }
    float getCurrent() 
    { 
        switch (assignment)
        {
            case Assignment::None: return smoothedValue.getCurrentValue(); 
            case Assignment::Pressure: return smoothedPressure.getCurrentValue();
            case Assignment::Timbre: return smoothedTimbre.getCurrentValue();
            default: jassertfalse;
        }
        return 0.0f;
    }
    void prepare (double sr) 
    { 
        sampleRate = sr;
        setControlSmoothing (controlSmoothingTimeMS);
        setPressureSmoothing (pressureSmoothingTimeMS);
        setTimbreSmoothing (timbreSmoothingTimeMS); 
    }
    void setState (juce::ValueTree mpeRoutingBranch) { mpeRouting = mpeRoutingBranch; }
    void setPressureSmoothing (float ms) 
    { 
        pressureSmoothingTimeMS = ms;
        smoothedPressure.reset (sampleRate, ms * 0.001); 
        smoothedPressure.setCurrentAndTargetValue (pressure);
    }
    void setTimbreSmoothing (float ms) 
    { 
        timbreSmoothingTimeMS = ms;
        smoothedTimbre.reset (sampleRate, ms * 0.001); 
        smoothedTimbre.setCurrentAndTargetValue (timbre);
    }
    void setControlSmoothing (float ms)
    {
        controlSmoothingTimeMS = ms;
        smoothedValue.reset (sampleRate, ms * 0.001);
        smoothedValue.setCurrentAndTargetValue (rangedParameter->convertFrom0to1 (rangedParameter->getValue()));
    }
private:
    juce::RangedAudioParameter* rangedParameter;
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::ValueTree mpeRouting;
    double sampleRate = 48000.0;
    juce::SmoothedValue<float> smoothedValue;
    juce::SmoothedValue<float> smoothedPressure;
    float pressureSmoothingTimeMS = 20.0f;
    float pressure = 0.0f;
    float timbre = 0.0f;
    juce::SmoothedValue<float> smoothedTimbre;
    float timbreSmoothingTimeMS = 20.0f;
    float controlSmoothingTimeMS = 20.0f;
    enum class Assignment
    {
        Pressure, 
        Timbre, 
        None
    };
    Assignment assignment;
    juce::ValueTree outputChannel;
    juce::Array<juce::Identifier> ids {id::OUTPUT_ONE, id::OUTPUT_TWO, id::OUTPUT_THREE};

    void checkAssignment()
    {
        auto timbreBranch = mpeRouting.getChildWithName (id::TIMBRE);
        for (auto id : ids)
        {
            if (rangedParameter == valueTreeState.getParameter (timbreBranch.getChildWithName (id)
                                                                            .getProperty (id::name).toString()))
            {
                outputChannel = timbreBranch.getChildWithName (id);
                assignment = Assignment::Timbre;
                return;
            }
        }
        auto pressureBranch = mpeRouting.getChildWithName (id::PRESSURE);
        for (auto id : ids)
        {
            if (rangedParameter == valueTreeState.getParameter (pressureBranch.getChildWithName (id)
                                                                              .getProperty (id::name).toString()))
            {
                outputChannel = pressureBranch.getChildWithName (id);
                assignment = Assignment::Pressure;
                return;
            }
        }

        assignment = Assignment::None;    
    }
    void setPressureInternal (float p)
    {
        jassert (outputChannel.isValid());
        float min = outputChannel.getProperty (id::lowerBound);
        float max = outputChannel.getProperty (id::upperBound);
        bool inv = outputChannel.getProperty (id::invertRange);
        auto value = inv ? juce::jmap (p, max, min) : 
                           juce::jmap (p, min, max);
        smoothedPressure.setTargetValue (rangedParameter->convertFrom0to1 (value));
    }
    void setTimbreInternal (float t)
    {
        jassert (outputChannel.isValid());
        float min = outputChannel.getProperty (id::lowerBound);
        float max = outputChannel.getProperty (id::upperBound);
        bool inv = outputChannel.getProperty (id::invertRange);
        auto value = inv ? juce::jmap (t, max, min) : 
                           juce::jmap (t, min, max);
        smoothedTimbre.setTargetValue (rangedParameter->convertFrom0to1 (value));
    }
    void parameterValueChanged (int parameterIndex, float newValue) override
    {
        juce::ignoreUnused (parameterIndex);
        if (assignment == Assignment::None)
            smoothedValue.setTargetValue (rangedParameter->convertFrom0to1 (newValue));
    }
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override { juce::ignoreUnused (parameterIndex, gestureIsStarting); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPESmoothedParameter)
};
class BufferedMPESmoothParameter
{
public:
    BufferedMPESmoothParameter (juce::RangedAudioParameter* p, 
                                juce::AudioProcessorValueTreeState& apvts, 
                                juce::ValueTree mpeRoutingBranch)
      : smoothedParameter (p, apvts, mpeRoutingBranch)
    {}
    void prepareToPlay (double sr, int blockSize)
    {
        smoothedParameter.prepare (sr);
        buffer.setSize (1, blockSize, false, false, true);
    }
    // call once per audio block
    void updateBuffer()
    {
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            auto* b = buffer.getWritePointer (0);
            b[i] = smoothedParameter.getNext();
        }
    }
    float getAt (int bufferIndex) { return buffer.getReadPointer (0)[bufferIndex]; }
    void allocate (int numSamples) { buffer.setSize (1, numSamples); }
    void noteOn (float mpePressure, float mpeTimbre) { smoothedParameter.noteOn (mpePressure, mpeTimbre); }
    void setTimbre (float newTimbre) { smoothedParameter.setTimbre (newTimbre); }
    void setPressure (float newPressure) { smoothedParameter.setPressure (newPressure); }
    void setState (juce::ValueTree routingBranch) { smoothedParameter.setState (routingBranch); }
    void setPressureSmoothing (float pressureSmoothing) { smoothedParameter.setPressureSmoothing (pressureSmoothing); }
    void setTimbreSmoothing (float timbreSmoothing) { smoothedParameter.setTimbreSmoothing (timbreSmoothing); }
private:
    MPESmoothedParameter smoothedParameter;
    juce::AudioBuffer<float> buffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferedMPESmoothParameter)
};

} //end namespace tp