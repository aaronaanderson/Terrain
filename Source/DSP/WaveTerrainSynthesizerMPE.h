#pragma once 

#include "WaveTerrainSynthesizer.h"
#include "../Utility/DefaultTreeGenerator.h"
namespace tp
{
class WaveTerrainSynthesizerMPE : public WaveTerrainSynthesizer,
                                  public juce::MPESynthesiser
{
public:
    WaveTerrainSynthesizerMPE (Parameters& p, 
                               MTSClient& mtsc, 
                               juce::ValueTree settings, 
                               juce::ValueTree& MPESettings,
                               juce::AudioProcessorValueTreeState& vts)
      : WaveTerrainSynthesizer (mtsc)
    {
        setPolyphony (15, p, settings, MPESettings, mtsClient, vts);
    }
    ~WaveTerrainSynthesizerMPE() override {}
    void prepareToPlay (double sr, int blockSize) override
    {
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto v = getVoice (i);
            auto trajectory = dynamic_cast<MPEVoice*> (v);
            if (trajectory != nullptr)
                trajectory->prepareToPlay (sr, blockSize);
        }
        setCurrentPlaybackSampleRate (sr);
    }
    void allocate (int maxNumSamples) override 
    { 
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto v = getVoice (i);
            auto trajectory = dynamic_cast<MPEVoice*> (v);
            if (trajectory != nullptr)
                trajectory->allocate (maxNumSamples);
        } 
    }
    void updateTerrain() override 
    {   
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto v = getVoice (i);
            auto trajectory = dynamic_cast<MPEVoice*> (v);
            if (trajectory != nullptr)
                trajectory->updateParameterBuffers();
        } 
    }
    juce::Array<VoiceInterface*> getVoices() override
    {
        juce::Array<VoiceInterface*> v;
        for(int i = 0; i < getNumVoices(); i++)
        {
            auto* vi = dynamic_cast<VoiceInterface*> (getVoice (i));
            v.add (vi);
        }

        return v;        
    }
    void setState (juce::ValueTree settings) override
    {
        jassert (settings.getType() == id::PRESET_SETTINGS);
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto v = getVoice (i);
            auto trajectory = dynamic_cast<MPEVoice*> (v);
            if (trajectory != nullptr)
                trajectory->setState (settings);
        }        
    }
    float getMaximumPressure()
    {
        float max = 0.0f;
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto v = getVoice (i);
            auto* t = dynamic_cast<MPEVoice*> (v);
            // if (!t->isActive()) break;
            if (t->getPressure() > max)
                max = t->getPressure();
        } 
        return max;
    }
    float getAverageTimbre()
    {
        float sum = 0.0f;
        int numActiveVoices = 0;
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto v = getVoice (i);
            auto* t = dynamic_cast<MPEVoice*> (v);
            if (t->getTimbre() <= 0.0f) break;
            numActiveVoices++;
            sum += t->getTimbre();
        } 
        if (numActiveVoices == 0) return 0.0f;
        return sum / static_cast<float> (numActiveVoices);
    }
    juce::ValueTree getVoicesState() { return voicesState; }
    void handleMidiEvent (const juce::MidiMessage& m) override
    {
        if (m.isPitchWheel())
        {
            int channel = m.getChannel();
            int pitchWheelValue = m.getPitchWheelValue();
            float normalizedPitchWheel = (pitchWheelValue - 8192) / 8192.0f;
            
            if (channel == 1)
                setGlobalPitchWheel (normalizedPitchWheel);
            else
                setChannelPitchWheel (channel, normalizedPitchWheel);
        }
        juce::MPESynthesiser::handleMidiEvent (m);
    }
    void setChannelPitchWheel (int channel, float pitchWheelNormalized)
    {
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto v = getVoice (i);
            auto* t = dynamic_cast<MPEVoice*> (v);
            if (t->getCurrentlyPlayingNote().midiChannel == channel)
            {
                t->setPitchWheel (pitchWheelNormalized);
                break;
            }
        } 
    }
    void setGlobalPitchWheel (float pitchWheelNormalized)
    {
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto v = getVoice (i);
            auto* t = dynamic_cast<MPEVoice*> (v);
            t->setGlobalPitchWheel (pitchWheelNormalized);
        }         
    }
private:
    juce::ValueTree voicesState = VoicesStateTree::create();
    void setPolyphony (int numVoices, 
                       Parameters& p, 
                       juce::ValueTree settingsBranch, 
                       juce::ValueTree& MPESettings,
                       MTSClient& mtsc, 
                       juce::AudioProcessorValueTreeState& vts)
    {
        jassert (numVoices > 0);
        clearVoices();
        juce::Array<VoiceInterface*> v;
        for (int i = 0; i < numVoices; i++)
        {
            MPEVoice* voice = new MPEVoice (p, settingsBranch, MPESettings, mtsc, vts, voicesState);
            addVoice (voice);
            VoiceInterface* interface = dynamic_cast<VoiceInterface*> (voice);
            v.add (interface);
        }

        if (voiceListener != nullptr)
            voiceListener->addVoices (v);        
    }
};
}