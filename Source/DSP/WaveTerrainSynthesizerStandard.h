#pragma once 

#include "WaveTerrainSynthesizer.h"

namespace tp
{
class WaveTerrainSynthesizerStandard : public WaveTerrainSynthesizer, 
                                       public juce::Synthesiser
{
public:
    WaveTerrainSynthesizerStandard (Parameters& p, 
                                    MTSClient& mtsc,
                                    juce::ValueTree settingsBranch)
      : WaveTerrainSynthesizer (mtsc), 
        terrain (p)
    {
        addSound (new DummySound());
        setPolyphony (24, p, settingsBranch, mtsClient);
    }
    ~WaveTerrainSynthesizerStandard() override {}
    void prepareToPlay (double sr, int blockSize) override
    {
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto v = getVoice (i);
            auto trajectory = dynamic_cast<StandardVoice*> (v);
            if (trajectory != nullptr)
                trajectory->prepareToPlay (sr, blockSize);
        }
        setCurrentPlaybackSampleRate (sr);
        
        terrain.prepareToPlay (sr, blockSize);
    }
    void allocate (int maxNumSamples) override
    { 
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto v = getVoice (i);
            auto trajectory = dynamic_cast<StandardVoice*> (v);
            if (trajectory != nullptr)
                trajectory->allocate (maxNumSamples);
        }
        terrain.allocate (maxNumSamples); 
    }
    void updateTerrain() override {terrain.updateParameterBuffers(); }
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
            auto trajectory = dynamic_cast<StandardVoice*> (v);
            if (trajectory != nullptr)
                trajectory->setState (settings);
        }        
    }
private:
    StandardTerrain terrain;
    void setPolyphony (int numVoices, 
                       Parameters& p, 
                       juce::ValueTree settingsBranch, 
                       MTSClient& mtsc)
    {
        jassert (numVoices > 0);
        clearVoices();
        juce::Array<VoiceInterface*> v;
        for (int i = 0; i < numVoices; i++)
        {
            auto* voice = new StandardVoice (terrain, p, settingsBranch, mtsc);
            addVoice (voice);
            auto* interface = dynamic_cast<VoiceInterface*> (voice);
            v.add (interface);
        }

        if (voiceListener != nullptr)
            voiceListener->addVoices (v);        
    }
};
}