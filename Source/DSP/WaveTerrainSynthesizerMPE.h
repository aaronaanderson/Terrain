#pragma once 

#include "WaveTerrainSynthesizer.h"

namespace tp
{
class WaveTerrainSynthesizerMPE : public WaveTerrainSynthesizer,
                                  public juce::MPESynthesiser
{
public:
    WaveTerrainSynthesizerMPE (Parameters& p, 
                               MTSClient& mtsc, 
                               juce::ValueTree settings, 
                               juce::AudioProcessorValueTreeState& vts)
      :  WaveTerrainSynthesizer (mtsc)
    {
        setPolyphony (15, p, settings, mtsClient, vts);
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
private:
    void setPolyphony (int numVoices, 
                       Parameters& p, 
                       juce::ValueTree settingsBranch, 
                       MTSClient& mtsc, 
                       juce::AudioProcessorValueTreeState& vts)
    {
        jassert (numVoices > 0);
        clearVoices();
        juce::Array<VoiceInterface*> v;
        for (int i = 0; i < numVoices; i++)
        {
            MPEVoice* voice = new MPEVoice (p, settingsBranch, mtsc, vts);
            addVoice (voice);
            VoiceInterface* interface = dynamic_cast<VoiceInterface*> (voice);
            v.add (interface);
        }

        if (voiceListener != nullptr)
            voiceListener->addVoices (v);        
    }
};
}