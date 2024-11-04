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
    WaveTerrainSynthesizer (MTSClient& mtsc)
      : mtsClient (mtsc)
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
    
    VoiceListener* voiceListener = nullptr;
    MTSClient& mtsClient;
};

}