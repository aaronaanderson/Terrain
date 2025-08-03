#pragma once

#include <juce_core/juce_core.h>

namespace tp {
struct ChannelData
{
    float pressure {0.0f};
    float timbre {0.0f};
    bool voiceActive {false};
    float rms {0.0f};
};

class MPEVoiceData
{
public:
    MPEVoiceData();

    // AUDIO THREAD!!
    void setPressureAT (float pressure, int channel);
    void setTimbreAT (float timbre, int channel);
    void setVoiceActiveAT (bool voiceActive, int channel);
    void setRMSAT (float rms, int channel);
    void setChannelDataAT (ChannelData cd, int channel);
    void publishAT(); // call this to update the message thread read data, at the end of the process block
    int getSizeAT() const;
    // MESSAGE THREAD!!
    juce::Array<ChannelData> getVoiceDataMT();

private:
    juce::Array<ChannelData> channelData[2]; // two arrays of channelData
    std::atomic<int> readIndex {1};
    std::atomic<int> writeIndex {0};
    juce::CriticalSection copyLock;
};

}