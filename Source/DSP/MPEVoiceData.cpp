#include "MPEVoiceData.h"

using namespace tp;



MPEVoiceData::MPEVoiceData()
{
    for (int i = 0; i < 2; i++)
        for (int v = 0; v < 17; v++)
            channelData[i].add (ChannelData());
}

void MPEVoiceData::setPressureAT (float pressure, int channel)
{
    jassert(channel >= 0 && channel < channelData[0].size());
    channelData[writeIndex.load (std::memory_order_relaxed)].getReference(channel).pressure = pressure;
}

void MPEVoiceData::setTimbreAT (float timbre, int channel)
{
    jassert(channel >= 0 && channel < channelData[0].size());
    channelData[writeIndex.load (std::memory_order_relaxed)].getReference(channel).timbre = timbre;
}

void MPEVoiceData::setVoiceActiveAT (bool voiceActive, int channel)
{
    jassert(channel >= 0 && channel < channelData[0].size());
    channelData[writeIndex.load (std::memory_order_relaxed)].getReference(channel).voiceActive = voiceActive;
}

void MPEVoiceData::setRMSAT (float rms, int channel)
{
    jassert(channel >= 0 && channel < channelData[0].size());
    channelData[writeIndex.load (std::memory_order_relaxed)].getReference(channel).rms = rms;
}

void MPEVoiceData::setChannelDataAT (ChannelData cd, int channel)
{
    jassert(channel >= 0 && channel < channelData[0].size());
    int writeIdx = writeIndex.load(std::memory_order_relaxed);
    channelData[writeIdx].getReference(channel) = cd;
}

void MPEVoiceData::publishAT()
{
    const int newRead = writeIndex.load (std::memory_order_relaxed);
    const int newWrite = (newRead + 1) % 2;

    readIndex.store(newRead, std::memory_order_release);
    writeIndex.store(newWrite, std::memory_order_relaxed);
}

juce::Array<ChannelData> MPEVoiceData::getVoiceDataMT()
{
    const juce::ScopedLock lock(copyLock); // Not RT-safe

    const int i = readIndex.load(std::memory_order_acquire);
    return channelData[i];
}