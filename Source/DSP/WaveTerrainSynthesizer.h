#pragma once

#include "../Parameters.h"
#include "DataTypes.h"
#include "Terrain.h"
#include "Trajectory.h"
namespace tp {
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