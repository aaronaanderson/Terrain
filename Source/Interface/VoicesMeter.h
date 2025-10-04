#pragma once 

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "LookAndFeel.h"

#include "../DSP/MPEVoiceData.h"

namespace ti
{
struct VoiceMeter : public juce::Component, 
                    private juce::Timer
{
    VoiceMeter (tp::MPEVoiceData& vd, 
                juce::ValueTree RoutingBranch)
      : voiceData (vd), 
        routingBranch (RoutingBranch)
    {
        startTimerHz (24);
    }

    void paint (juce::Graphics& g) override
    {
            paintArc (g);
    }
    void setMPEChannel (juce::Identifier mpeCh)    { mpeChannel = mpeCh; }
    void setOutputID (juce::Identifier output)     { outputID = output; }
    void setRoutingState (juce::ValueTree routing) { routingBranch = routing; }
private:
    void timerCallback() override { repaint(); }
    tp::MPEVoiceData& voiceData;
    juce::ValueTree routingBranch;
    juce::Identifier mpeChannel;
    juce::Identifier outputID;
    
    void paintArc (juce::Graphics& g)
    {
        auto bounds = getLocalBounds().reduced (10);
        juce::Slider s;
        auto rotaryStartAngle = s.getRotaryParameters().startAngleRadians;
        auto rotaryEndAngle = s.getRotaryParameters().endAngleRadians; 
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        // auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = juce::jmin (10.0f, radius * 0.5f);
        auto arcRadius = radius - lineW * 0.5f;
    
        juce::Path backgroundArc;
        backgroundArc.addCentredArc (bounds.toFloat().getCentreX(),
                                     bounds.toFloat().getCentreY(),
                                     arcRadius,
                                     arcRadius,
                                     0.0f,
                                     rotaryStartAngle,
                                     rotaryEndAngle,
                                     true);
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        g.setColour (laf->getBackgroundColour());
        g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::mitered, juce::PathStrokeType::square));

        auto voicesData = voiceData.getVoiceDataMT();
        for (int i = 0; i < voicesData.size(); i++)
        {
            // if (!voicesState.getChild (i).getProperty (id::voiceActive)) continue;
            if (!voicesData[i].voiceActive) { continue; }

            float x = 0.0f;
            if (mpeChannel == id::PRESSURE) x = voicesData[i].pressure;
            if (mpeChannel == id::TIMBRE) x = voicesData[i].timbre;
            auto curvedX = curveValue (x, 
                                       (float)routingBranch.getChildWithName (mpeChannel).getChildWithName (outputID).getProperty (id::curve),
                                       (float)routingBranch.getChildWithName (mpeChannel).getChildWithName (outputID).getProperty (id::handleOne),
                                       (float)routingBranch.getChildWithName (mpeChannel).getChildWithName (outputID).getProperty (id::handleTwo));
            auto p = normalToArc (curvedX, rotaryStartAngle, rotaryEndAngle, arcRadius, bounds);
            auto width = lineW * 0.6f;
            g.setColour (laf->getAccentColour());
            g.fillEllipse (juce::Rectangle<float> (width, width).withCentre (p));
        }
        
    }
   
    juce::Point<float> normalToArc (float normalPosition, float startAngle, float endAngle, float radius, juce::Rectangle<int> bounds)
    {
        auto angle = startAngle + normalPosition * (endAngle - startAngle);
        return juce::Point<float> (bounds.toFloat().getCentreX() + radius * std::cos (angle - juce::MathConstants<float>::halfPi),
                                   bounds.toFloat().getCentreY() + radius * std::sin (angle - juce::MathConstants<float>::halfPi));
    }
    float curveValue (const float linearValue, const float curve, const float min, const float max)
    {
    
        float curvedValue = (float)std::pow (linearValue, 1.0f / curve);
        return juce::jmap (curvedValue, min, max);
    }
};
}