#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../Parameters.h"
#include "ADSR.h"

#include <PerlinNoise/PerlinNoise.hpp>
namespace tp {
struct Point 
{  
    Point (float xp = 0.0f, float yp = 0.0f) : x(xp), y(yp) {}
    float x; float y; 

    Point operator+(const Point& other) { return Point(this->x + other.x, this->y + other.y); }
    Point operator*(float scalar) { return Point(this->x * scalar, this->y * scalar); }
};
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
    ~SmoothedParameter() override
    {
        rangedParameter->removeListener (this);
    }
    void noteOn() 
    {
        smoothedValue.setCurrentAndTargetValue (rangedParameter->convertFrom0to1 (rangedParameter->getValue())); 
    }
    float getNext() 
    {
        return smoothedValue.getNextValue();
    }
    float getCurrent()
    {
        return smoothedValue.getCurrentValue();
    }
    void prepare (double sampleRate) 
    { 
        smoothedValue.reset (sampleRate, 0.02f);
    }

private:
    juce::RangedAudioParameter* rangedParameter;
    juce::SmoothedValue<float> smoothedValue;

    void parameterValueChanged (int parameterIndex, float newValue) override
    {
        juce::ignoreUnused (parameterIndex);
        smoothedValue.setTargetValue (rangedParameter->convertFrom0to1 (newValue));
    }
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override { juce::ignoreUnused (parameterIndex, gestureIsStarting); }
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
        buffer.resize (blockSize);
    }
    // call once per audio block
    void updateBuffer()
    {
        for (auto& s : buffer)
            s = smoothedParameter.getNext();
    }
    float getAt (int bufferIndex) { return buffer[bufferIndex]; }
private:
    SmoothedParameter smoothedParameter;
    juce::Array<float> buffer;
};
class Terrain : public juce::SynthesiserSound
{
public:
    Terrain (Parameters& p)
      : parameters (p), 
        modA (p.terrainModA), 
        modB (p.terrainModB), 
        modC (p.terrainModC), 
        modD (p.terrainModD), 
        saturation (p.terrainSaturation)
    {}
    bool appliesToNote (int midiNoteNumber) override { juce::ignoreUnused (midiNoteNumber); return true; }
    bool appliesToChannel (int midiChannel) override { juce::ignoreUnused (midiChannel); return true; }
    void prepareToPlay(double sampleRate, int blockSize)
    {
        modA.prepareToPlay (sampleRate, blockSize);
        modB.prepareToPlay (sampleRate, blockSize);
        modC.prepareToPlay (sampleRate, blockSize);
        modD.prepareToPlay (sampleRate, blockSize);
        saturation.prepareToPlay (sampleRate, blockSize);
    }
    void updateParameterBuffers()
    {
        modA.updateBuffer();
        modB.updateBuffer();
        modC.updateBuffer();
        modD.updateBuffer();
        saturation.updateBuffer();
    }
    float sampleAt (Point p, int bufferIndex)
    {
        auto m = getModSet (bufferIndex);
        float output = 0.0f;
        switch (*parameters.currentTerrain)
        {
            case 0:
                output = std::sin(p.x * 6.0f * (m.a + 0.5f)) * std::sin(p.y * 6.0f * (m.b + 0.5f));
                break;
            case 1:
                output = std::sin((p.x * juce::MathConstants<float>::twoPi) * (p.x * 3.0f * m.a) + (m.b * juce::MathConstants<float>::twoPi)) * 
                       std::sin((p.y * juce::MathConstants<float>::twoPi) * (p.y * 3.0f * m.a) + (m.b * -juce::MathConstants<float>::twoPi));
                break;
            case 2:
                output = std::cos(dfc (p) * juce::MathConstants<float>::twoPi * (m.a * 5.0f + 1.0f) + (m.b * juce::MathConstants<float>::twoPi));
                break;
            case 3:
                output = (1.0f - (p.x * p.y)) * std::cos((m.a * 14.0f + 1.0f) * (1.0f - p.x * p.y));
                break;
            case 4:
            {
                float c = m.a * 0.5f + 0.25f;
                float d = m.b * 16.0f + 4.0f;  
                output = c * p.x * std::cos((1.0f - c) * d * juce::MathConstants<float>::pi * p.x * p.y)  +  (1.0f - c) * p.y * cos(c * d * juce::MathConstants<float>::pi * p.x * p.y);
            }
                break;
            case 5:
            {
                float aa = m.a * 4.0f + 1.0f;
                float bb = m.b * 4.0f + 1.0f;
                float cc = m.c * 0.8f + 0.1f;
                output = ((std::pow (aa * p.x, 2.0f) + std::pow (bb * p.y, 2.0f)) * 
                           std::pow (cc, (std::pow (4.0f * p.x, 2.0f) + 
                                          std::pow (4.0f * p.y, 2.0f)))) * 2.0f - 1.0f;
            }
                break;
            default:
            jassertfalse;
        }

        return saturate (output, saturation.getAt (bufferIndex));
    }
private:
    Parameters& parameters;
    BufferedSmoothParameter modA, modB, modC, modD, saturation;

    const ModSet getModSet (int index)
    {
        return ModSet (modA.getAt (index), modB.getAt (index), 
                       modC.getAt (index), modD.getAt (index));
    }
    // distance from center
    inline float dfc (Point p) { return std::sqrt (p.x * p.x + p.y * p.y); }
    float saturate (float signal, float scale)
    {
        return juce::dsp::FastMathApproximations::tanh<float> (signal * scale * 1.31303528551f);
    }
};
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

            if (terrain != nullptr)
            {
                float outputSample = terrain->sampleAt (point, i);
                history.feedNext (point, outputSample);
                o[i] += outputSample * static_cast<float> (envelope.calculateNext()) * 0.1f;
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
class WaveTerrainSynthesizer : public juce::Synthesiser, 
                               private juce::ValueTree::Listener
{
public:
    WaveTerrainSynthesizer (Parameters& p)
    {
        setPolyphony (24, p);
        addSound (new Terrain (p));
    }
    void prepareToPlay (double sr, int blockSize)
    {
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto v = getVoice (i);
            auto trajectory = dynamic_cast<Trajectory*> (v);
            if (trajectory != nullptr)
                trajectory->prepareToPlay (sr, blockSize);
        }
        setCurrentPlaybackSampleRate (sr);
        
        jassert (getNumSounds() == 1);
        auto terrain = dynamic_cast<Terrain*> (getSound (0).get());
        jassert (terrain != nullptr);
        terrain->prepareToPlay (sr, blockSize);
    }
    // must be called once per buffer
    void updateTerrain()
    {
        jassert (getNumSounds() == 1);
        auto terrain = dynamic_cast<Terrain*> (getSound (0).get());
        jassert (terrain != nullptr);
        terrain->updateParameterBuffers();
    }
    struct VoiceListener
    {
        virtual ~VoiceListener() {}
        virtual void voicesReset (juce::Array<juce::SynthesiserVoice*> newVoice) = 0;
    };
    void setVoiceListener(VoiceListener* vl) { voiceListener = vl; }
    juce::Array<juce::SynthesiserVoice*> getVoices()
    {
        juce::Array<juce::SynthesiserVoice*> v;
        for(int i = 0; i < getNumVoices(); i++)
            v.add (getVoice (i));

        return v;
    }
private:
    VoiceListener* voiceListener = nullptr;
    void setPolyphony (int newPolyphony, Parameters& p)
    {
        jassert (newPolyphony > 0);
        clearVoices();
        juce::Array<juce::SynthesiserVoice*> v;
        for (int i = 0; i < newPolyphony; i++)
            v.add (addVoice (new Trajectory (p)));

        if (voiceListener != nullptr)
            voiceListener->voicesReset (v);
    }
};
}