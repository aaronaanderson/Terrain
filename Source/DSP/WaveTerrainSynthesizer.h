#pragma once

#include <MTS-ESP/Client/libMTSClient.h>
#include "../Parameters.h"
#include "DataTypes.h"
#include "Terrain.h"
#include "StandardVoice.h"
#include "MPEVoice.h"
namespace tp {
class WaveTerrainSynthesizer
{
public:
    WaveTerrainSynthesizer (Parameters& p)
      : terrain (p)
    {
        mtsClient = MTS_RegisterClient();
    }
    virtual ~WaveTerrainSynthesizer()
    {
        MTS_DeregisterClient (mtsClient);
    }

    virtual void prepareToPlay (double sampleRate, int blockSize) = 0;
    virtual void allocate (int maxBlockSize) = 0;
    virtual void updateTerrain() = 0;
    virtual juce::Array<VoiceInterface*> getVoices() = 0;
    virtual void setState (juce::ValueTree settingsBranch) = 0;
    struct VoiceListener
    {
        virtual ~VoiceListener() {}
        virtual void voicesReset (juce::Array<VoiceInterface*> newVoice) = 0;       
    };
    void setVoiceListener (VoiceListener* l) { voiceListener = l; }
    bool getMTSConnectionStatus() { return MTS_HasMaster (mtsClient); }
    juce::String getTuningSystemName() { return MTS_GetScaleName (mtsClient); }
protected:
    Terrain terrain;
    VoiceListener* voiceListener = nullptr;
    MTSClient* mtsClient = nullptr;
};
class WaveTerrainSynthesizerStandard : public WaveTerrainSynthesizer, 
                                       public juce::Synthesiser
{
public:
    WaveTerrainSynthesizerStandard (Parameters& p, juce::ValueTree settingsBranch)
      : WaveTerrainSynthesizer (p)
    {
        addSound (new DummySound());
        setPolyphony (24, p, settingsBranch, *mtsClient);
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
    void allocate (int maxNumSamples) override { terrain.allocate (maxNumSamples); }
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
            voiceListener->voicesReset (v);        
    }
};
class WaveTerrainSynthesizerMPE : public WaveTerrainSynthesizer,
                                  public juce::MPESynthesiser
{

};

}