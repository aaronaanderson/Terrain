#pragma once 

#include "WaveTerrainSynthesizer.h"
#include "../Utility/DefaultTreeGenerator.h"
#include "morphlib/Synthesizer.h"
namespace tp
{
class WaveTerrainSynthesizerMPE : public WaveTerrainSynthesizer,
                                  //public juce::MPESynthesiser
                                  public morph::Synthesizer
{
public:
    // ERASE THIS
    virtual void prepareToPlayERASE (double sr, int blockSize) { juce::ignoreUnused (sr, blockSize); }
    virtual void allocateERASE (int maxBlockSize) { juce::ignoreUnused (maxBlockSize); }
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
    juce::Array<VoiceInterface*> getVoices() override // TODO: delete this function
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

    juce::ValueTree getVoicesState() { return voicesState; }

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