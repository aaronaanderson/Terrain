#pragma once 

#include "MPEVoice.h"
#include "../Utility/DefaultTreeGenerator.h"
#include "morphlib/Synthesizer.h"

namespace tp
{
class WaveTerrainSynthesizerMPE : public morph::Synthesizer
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
                               juce::AudioProcessorValueTreeState& vts, 
                               MPEVoiceData& vd)
        : voiceData (vd)
    {
        setPolyphony (15, p, settings, MPESettings, mtsc, vts, vd);
    }
    ~WaveTerrainSynthesizerMPE() override {}

    void updateTerrain() 
    {   
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto v = getVoice (i);
            auto trajectory = dynamic_cast<MPEVoice*> (v);
            if (trajectory != nullptr)
                trajectory->updateParameterBuffers();
        } 
    }

    void setState (juce::ValueTree settings)
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

    void updateVoiceData()
    {
        for (int i = 0; i < getNumVoices(); i++)
        {
            auto voice = dynamic_cast<MPEVoice*> (getVoice (i));
            if (voice->isActive())
            {
                tp::ChannelData cd {voice->getPressure(), voice->getTimbre(), true, voice->getRMS()};
                voiceData.setChannelDataAT ( cd, i);
            } else {
                tp::ChannelData cd {0.0f, 0.0f, false, 0.0f};
                voiceData.setChannelDataAT ( cd, i);
            }
        }
    }

private:
    tp::MPEVoiceData& voiceData;
    void setPolyphony (int numVoices, 
                       Parameters& p, 
                       juce::ValueTree settingsBranch, 
                       juce::ValueTree& MPESettings,
                       MTSClient& mtsc, 
                       juce::AudioProcessorValueTreeState& vts, 
                       MPEVoiceData& vd)
    {
        jassert (numVoices > 0);
        clearVoices();
        for (int i = 0; i < numVoices; i++)
        {
            MPEVoice* voice = new MPEVoice (p, settingsBranch, MPESettings, mtsc, vts, vd);
            addVoice (voice);
        }

               
    }
};
}
