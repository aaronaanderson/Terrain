#pragma once

#include <MTS-ESP/Client/libMTSClient.h>
#include "../Parameters.h"
#include "DataTypes.h"
#include "Terrain.h"
#include "Trajectory.h"
namespace tp {
class WaveTerrainSynthesizer : public juce::Synthesiser
{
public:
    WaveTerrainSynthesizer (Parameters& p, juce::ValueTree settings)
    {
        mtsClient = MTS_RegisterClient();
        addSound (new Terrain (p));
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
    void allocate (int maxNumSamples)
    {
        jassert (getNumSounds() == 1);
        auto terrain = dynamic_cast<Terrain*> (getSound (0).get());
        jassert (terrain != nullptr);
        terrain->allocate (maxNumSamples);
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
            auto trajectory = dynamic_cast<Trajectory*> (v);
            if (trajectory != nullptr)
                trajectory->setState (settings);
        }
    }
private:
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
            v.add (addVoice (new Trajectory (p, settings, mtsc)));

        if (voiceListener != nullptr)
            voiceListener->voicesReset (v);
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveTerrainSynthesizer)
};
}