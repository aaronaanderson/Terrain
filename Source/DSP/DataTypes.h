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
    void prepare (double sampleRate) { smoothedValue.reset (sampleRate, 0.02f); }

private:
    juce::RangedAudioParameter* rangedParameter;
    juce::SmoothedValue<float> smoothedValue;

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
                setPressureInternal (mpePressure); 
            break;
            case Assignment::Timbre:
                setTimbreInternal (mpeTimbre);
            break;
            default:
                jassertfalse;
        }
    }
    void setPressure (float pressure) 
    { 
        if (assignment != Assignment::Pressure) return;
        setPressureInternal (pressure); 
    };
    void setTimbre (float timbre)
    {
        if (assignment != Assignment::Timbre) return;
        setTimbreInternal (timbre); 
    }
    float getNext() { return smoothedValue.getNextValue(); }
    float getCurrent() { return smoothedValue.getCurrentValue(); }
    void prepare (double sampleRate) { smoothedValue.reset (sampleRate, 0.02f); }
    void setState (juce::ValueTree mpeRoutingBranch) { mpeRouting = mpeRoutingBranch; }
private:
    juce::RangedAudioParameter* rangedParameter;
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::ValueTree mpeRouting;
    juce::SmoothedValue<float> smoothedValue;
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
                std::cout << outputChannel.toXmlString() << std::endl;
                return;
            }
        }

        assignment = Assignment::None;    
    }
    void setPressureInternal (float pressure)
    {
        jassert (outputChannel.isValid());
        float min = outputChannel.getProperty (id::lowerBound);
        float max = outputChannel.getProperty (id::upperBound);
        auto value = juce::jmap (pressure, min, max);
        smoothedValue.setCurrentAndTargetValue (rangedParameter->convertFrom0to1 (value));
    }
    void setTimbreInternal (float timbre)
    {
        jassert (outputChannel.isValid());
        float min = outputChannel.getProperty (id::lowerBound);
        float max = outputChannel.getProperty (id::upperBound);
        auto value = juce::jmap (timbre, min, max);
        smoothedValue.setCurrentAndTargetValue (rangedParameter->convertFrom0to1 (value));
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
private:
    MPESmoothedParameter smoothedParameter;
    juce::AudioBuffer<float> buffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferedMPESmoothParameter)
};

} //end namespace tp