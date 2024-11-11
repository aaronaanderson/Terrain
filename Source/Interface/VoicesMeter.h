#pragma once 

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "LookAndFeel.h"

namespace ti
{
struct VoiceMeter : public juce::Component, 
                    private juce::Timer
{
    VoiceMeter (juce::ValueTree voicesStateTree, 
                juce::ValueTree RoutingBranch)
      : voicesState (voicesStateTree), 
        routingBranch (RoutingBranch)
    {
        startTimerHz (24);
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds();
        if (b.getHeight() > b.getWidth() * 1.5)
            paintVertical (g);
        else if (b.getHeight() * 2 >= b.getWidth())
            paintArc (g);
        else
            paintHorizontal (g);
    }
    void setMPEChannel (juce::Identifier mpeCh) { mpeChannel = mpeCh; }
    void setOutputID (juce::Identifier output) { outputID = output; }
    void setRoutingState (juce::ValueTree routing) { routingBranch = routing; }
private:
    void timerCallback() override { repaint(); }
    juce::ValueTree voicesState;
    juce::ValueTree routingBranch;
    juce::Identifier mpeChannel;
    juce::Identifier outputID;
    
    void paintHorizontal (juce::Graphics& g)
    {
        auto b = getLocalBounds();
        auto sliderRect = b.reduced (4);
        sliderRect = sliderRect.withHeight (juce::jmin (sliderRect.getHeight(), 16));
        sliderRect = sliderRect.withCentre (b.getCentre());
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        g.setColour (laf->getBackgroundColour());
        g.fillRect (sliderRect.toFloat());

        float width = sliderRect.getHeight() * 0.6f;
        for (int i = 0; i < voicesState.getNumChildren(); i++)
        {
            if (!(bool)voicesState.getChild (i).getProperty (id::voiceActive)) continue;
            
            g.setColour (laf->getAccentColour());
            float x = 0.0f;
            if (mpeChannel == id::PRESSURE) x = voicesState.getChild (i).getProperty (id::voicePressure);
            if (mpeChannel == id::TIMBRE) x = voicesState.getChild (i).getProperty (id::voiceTimbre);                       
            auto curvedX = curveValue (x, 
                                       (float)routingBranch.getChildWithName (mpeChannel).getChildWithName (outputID).getProperty (id::curve),
                                       (float)routingBranch.getChildWithName (mpeChannel).getChildWithName (outputID).getProperty (id::handleOne),
                                       (float)routingBranch.getChildWithName (mpeChannel).getChildWithName (outputID).getProperty (id::handleTwo));
            
            x = juce::jmap (curvedX, width, (float)sliderRect.getWidth());
            auto thumbRect = juce::Rectangle<float> ((float)width, (float)width);
            g.fillEllipse (thumbRect.withCentre({x, (float)sliderRect.getCentreY()}));
        }
    }
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

        for (int i = 0; i < voicesState.getNumChildren(); i++)
        {
            if (!voicesState.getChild (i).getProperty (id::voiceActive)) continue;
            float x = 0.0f;
            if (mpeChannel == id::PRESSURE) x = voicesState.getChild (i).getProperty (id::voicePressure);
            if (mpeChannel == id::TIMBRE) x = voicesState.getChild (i).getProperty (id::voiceTimbre);
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
    void paintVertical (juce::Graphics& g)
    {
        auto b = getLocalBounds();
        auto sliderRect = b.reduced (4);
        sliderRect = sliderRect.withWidth (juce::jmin (sliderRect.getWidth(), 16));
        sliderRect = sliderRect.withCentre (b.getCentre());
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        g.setColour (laf->getBackgroundColour());
        g.fillRect (sliderRect.toFloat());

        float width = sliderRect.getWidth() * 0.6f;
        for (int i = 0; i < voicesState.getNumChildren(); i++)
        {
            if (!voicesState.getChild (i).getProperty (id::voiceActive)) continue;
            g.setColour (laf->getAccentColour());
            float x = 0.0f;
            if (mpeChannel == id::PRESSURE) x = voicesState.getChild (i).getProperty (id::voicePressure);
            if (mpeChannel == id::TIMBRE) x = voicesState.getChild (i).getProperty (id::voiceTimbre);
            auto curvedX = curveValue (x, 
                                       (float)routingBranch.getChildWithName (mpeChannel).getChildWithName (outputID).getProperty (id::curve),
                                       (float)routingBranch.getChildWithName (mpeChannel).getChildWithName (outputID).getProperty (id::handleOne),
                                       (float)routingBranch.getChildWithName (mpeChannel).getChildWithName (outputID).getProperty (id::handleTwo));            
            x = juce::jmap (curvedX, 0.0f, (float)sliderRect.getHeight() - width);
            auto thumbRect = juce::Rectangle<float> ((float)width, (float)width);
            g.fillEllipse (thumbRect.withCentre({(float)sliderRect.getCentreX(), sliderRect.getHeight() - x}));
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