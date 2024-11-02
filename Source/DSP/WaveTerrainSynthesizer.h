#pragma once

#include <MTS-ESP/Client/libMTSClient.h>
#include "../Parameters.h"
#include "DataTypes.h"
#include "Terrain.h"
#include "StandardVoice.h"
namespace tp {
class WaveTerrainSynthesizerBase
{
public:
    WaveTerrainSynthesizerBase()
    {
        mtsClient = MTS_RegisterClient();
    }
    virtual ~WaveTerrainSynthesizerBase()
    {
        MTS_DeregisterClient (mtsClient);
    }

    virtual void prepareToPlay (double sampleRate, int blockSize) = 0;
    virtual void allocate (int maxBlockSize) = 0;
    virtual void updateTerrain() = 0;
    virtual juce::Array<VoiceInterface> getVoices() = 0;
    struct VoiceListener
    {
        virtual ~VoiceListener() {}
        virtual void voicesReset (juce::Array<VoiceInterface*> newVoice) = 0;       
    };
    void setVoiceListener (VoiceListener* l) { voiceListener = l; }
    bool getMTSConnectionStatus() { return MTS_HasMaster (mtsClient); }
    juce::String getTuningSystemName() { return MTS_GetScaleName (mtsClient); }
protected:
    VoiceListener* voiceListener = nullptr;
private:
    MTSClient* mtsClient = nullptr;
};

class WaveTerrainSynthesizer : public juce::Synthesiser
{
public:
    WaveTerrainSynthesizer (Parameters& p, juce::ValueTree settings)
      : terrain (p)
    {
        mtsClient = MTS_RegisterClient();

        // addSound (new Terrain (p));
        addSound (new DummySound());
        setPolyphony (24, p, settings, *mtsClient);
    }
    ~WaveTerrainSynthesizer()
    {
        MTS_DeregisterClient (mtsClient);
    }
    void prepareToPlay (double sr, int blockSize)
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
    void allocate (int maxNumSamples) { terrain.allocate(maxNumSamples); }
    // must be called once per buffer
    void updateTerrain() { terrain.updateParameterBuffers(); }
    struct VoiceListener
    {
        virtual ~VoiceListener() {}
        virtual void voicesReset (juce::Array<juce::SynthesiserVoice*> newVoice) = 0;
    };
    void setVoiceListener (VoiceListener* vl) { voiceListener = vl; }
    juce::Array<juce::SynthesiserVoice*> getVoices()
    {
        juce::Array<juce::SynthesiserVoice*> v;
        for(int i = 0; i < getNumVoices(); i++)
            v.add (getVoice (i));

        return v;
    }
    void setState (juce::ValueTree settings)
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
    bool getMTSConnectionStatus() { return MTS_HasMaster (mtsClient); }
    juce::String getTuningSystemName() { return MTS_GetScaleName (mtsClient); }
private:
    Terrain terrain;
    VoiceListener* voiceListener = nullptr;
    MTSClient* mtsClient = nullptr;
    void setPolyphony (int newPolyphony, 
                       Parameters& p, 
                       juce::ValueTree settings, 
                       MTSClient& mtsc)
    {
        jassert (newPolyphony > 0);
        clearVoices();
        juce::Array<juce::SynthesiserVoice*> v;
        for (int i = 0; i < newPolyphony; i++)
            v.add (addVoice (new StandardVoice (terrain, p, settings, mtsc)));

        if (voiceListener != nullptr)
            voiceListener->voicesReset (v);
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveTerrainSynthesizer)
};

class WaveTerrainSynthesizerStandard : public WaveTerrainSynthesizerBase, 
                                       public juce::Synthesiser
{

};
class WaveTerrainSynthesizerMPE : public WaveTerrainSynthesizerBase,
                                  public juce::MPESynthesiser
{

};

}