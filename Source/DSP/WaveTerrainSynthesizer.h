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
    WaveTerrainSynthesizer (Parameters& p, MTSClient& mtsc)
      : terrain (p), 
        mtsClient (mtsc)
    {}
    virtual ~WaveTerrainSynthesizer(){}
    virtual void prepareToPlay (double sampleRate, int blockSize) = 0;
    virtual void allocate (int maxBlockSize) = 0;
    virtual void updateTerrain() = 0;
    virtual juce::Array<VoiceInterface*> getVoices() = 0;
    virtual void setState (juce::ValueTree settingsBranch) = 0;
    struct VoiceListener
    {
        virtual ~VoiceListener() {}
        virtual void addVoices (juce::Array<VoiceInterface*> newVoice) = 0;
        virtual void resetVoices() = 0;   
    };
    void setVoiceListener (VoiceListener* l) { voiceListener = l; }
protected:
    Terrain terrain;
    VoiceListener* voiceListener = nullptr;
    MTSClient& mtsClient;
};
class WaveTerrainSynthesizerStandard : public WaveTerrainSynthesizer, 
                                       public juce::Synthesiser
{
public:
    WaveTerrainSynthesizerStandard (Parameters& p, 
                                   MTSClient& mtsc,
                                   juce::ValueTree settingsBranch)
      : WaveTerrainSynthesizer (p, mtsc)
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
            voiceListener->addVoices (v);        
    }
};
class WaveTerrainSynthesizerMPE : public WaveTerrainSynthesizer,
                                  public juce::MPESynthesiser
{
public:
    WaveTerrainSynthesizerMPE (Parameters& p, 
                               MTSClient& mtsc, 
                               juce::ValueTree settings)
      :  WaveTerrainSynthesizer (p, mtsc)
    {
        setPolyphony (15, p, settings, mtsClient);
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
            auto trajectory = dynamic_cast<MPEVoice*> (v);
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
            MPEVoice* voice = new MPEVoice (terrain, p, settingsBranch, mtsc);
            addVoice (voice);
            VoiceInterface* interface = dynamic_cast<VoiceInterface*> (voice);
            v.add (interface);
        }

        if (voiceListener != nullptr)
            voiceListener->addVoices (v);        
    }
};
}