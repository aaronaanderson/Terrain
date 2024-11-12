#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "LookAndFeel.h"

namespace ti
{
struct Handle : public juce::Component
{
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds();
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());

        if (!mouseOver) b = b.reduced (3);
       
        g.setColour (laf->getAccentColour());
        g.fillEllipse (b.toFloat());
        b = b.reduced (2);
        g.setColour (laf->getBaseColour());
        g.fillEllipse (b.toFloat());
    }
    void mouseEnter (const juce::MouseEvent& e) override
    {
        juce::ignoreUnused (e);
        mouseOver = true;
        repaint();
    }
    void mouseExit (const juce::MouseEvent& e) override
    {
        juce::ignoreUnused (e);
        mouseOver = false;
        repaint();
    }
    void mouseDown (const juce::MouseEvent& e) override 
    {
        juce::ignoreUnused (e);
        preDragPosition = normalizedPosition;
    }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        float normilazationScalar = (1.0f / this->getParentHeight());
        float offset = e.getDistanceFromDragStartY() * normilazationScalar * -1.0f;
        normalizedPosition = juce::jlimit (0.0f, 1.0f, preDragPosition + offset);

        if (listener != nullptr) listener->onHandleDrag (this);
    }
    struct Listener 
    {
        virtual void onHandleDrag (const Handle*) = 0;
    };
    void setListener (Listener* l) { listener = l; }
    void setNormalizedPosition (const float newPosition) 
    {
        jassert (newPosition >= 0.0f && newPosition <= 1.0f);
        normalizedPosition = newPosition;
    }
    float getNormalizedPosition() const { return normalizedPosition; }
private:
    bool mouseOver = false;
    Listener* listener = nullptr;
    float normalizedPosition = 0.5f;
    float preDragPosition = 0.0f;
};
struct RoutingCurve : public juce::Component, 
                      private Handle::Listener
{
    RoutingCurve (juce::ValueTree routingChannelBranch)
      : routingChannel (routingChannelBranch)
    {
        handleOne.setNormalizedPosition (routingChannel.getProperty (id::handleOne));
        handleOne.setListener (this);
        addAndMakeVisible (handleOne);
        
        handleTwo.setNormalizedPosition (routingChannel.getProperty (id::handleTwo));
        handleTwo.setListener (this);
        addAndMakeVisible (handleTwo);

        curve = routingChannel.getProperty (id::curve);
    }
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds();
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        g.fillAll (laf->getBackgroundColour());
        g.setColour (laf->getBackgroundDark());
        g.drawRect (b.toFloat(), 2.0f);

        int adjustedHeight = bounds.getHeight() - (handleSize);
        auto yOne = adjustedHeight - (handleOne.getNormalizedPosition() * adjustedHeight) + (handleSize / 2);
        auto yTwo = adjustedHeight - (handleTwo.getNormalizedPosition() * adjustedHeight) + (handleSize / 2);

        juce::Path curvePath;
        float thicc = mouseOver ? 4.0f : 2.0f;
        for (int i = 0; i < b.getWidth(); i++)
        {
            float normalX = juce::jmap (static_cast<float> (i), 0.0f, static_cast<float> (b.getWidth()) - 1.0f, 0.0f, 1.0f);
            auto normalY = static_cast<float> (std::pow (normalX, 1.0f / curve));
  
            juce::Point<float> nextPoint;
            nextPoint.setX (juce::jmap (normalX, 0.0f, 1.0f, float(handleSize / 2), (float)bounds.getWidth() - (handleSize / 2)));
            nextPoint.setY (juce::jmap (normalY, 0.0f, 1.0f, yOne, yTwo));
            if (i == 0)
                curvePath.startNewSubPath (nextPoint);
            else
                curvePath.lineTo (nextPoint); 
        }  
        g.setColour (laf->getAccentColour());
        g.strokePath (curvePath, juce::PathStrokeType (thicc));
    }
    void resized() override
    {
        bounds = getLocalBounds().reduced (2);
        
        juce::Rectangle<int> handleBounds {handleSize, handleSize};
        int adjustedHeight = bounds.getHeight() - handleSize;
        auto y = adjustedHeight - static_cast<int> (adjustedHeight * handleOne.getNormalizedPosition());
        handleOne.setBounds (handleBounds.withPosition ({bounds.getX(), y}));
        y = adjustedHeight - static_cast<int> (adjustedHeight * handleTwo.getNormalizedPosition());
        handleTwo.setBounds (handleBounds.withPosition ({bounds.getWidth() - 20, 
                                                         y}));
    }
    void mouseEnter (const juce::MouseEvent& e) override
    {
        juce::ignoreUnused (e);
        mouseOver = true;
        repaint();
    }
    void mouseExit (const juce::MouseEvent& e) override
    {
        juce::ignoreUnused (e);
        mouseOver = false;
        repaint();
    }
    void mouseDown (const juce::MouseEvent& e) override
    {
        juce::ignoreUnused (e);
        preDragPosition = preCurve;
    }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        juce::ignoreUnused (e);
        float normalizationScalar = (1.0f / this->getParentHeight());
        preCurve = e.getDistanceFromDragStartY() * normalizationScalar * 8.0f;
        if (handleOne.getNormalizedPosition() < handleTwo.getNormalizedPosition()) preCurve *= -1.0f;
        preCurve = juce::jlimit (0.0f, 8.0f, preDragPosition + preCurve);
        curve = (float)std::pow (2, preCurve - 4.0f);
        routingChannel.setProperty (id::curve, curve, nullptr);

        repaint();
    }
private:
    juce::ValueTree routingChannel;
    Handle handleOne, handleTwo;
    juce::Rectangle<int> bounds;
    int handleSize = 20;

    bool mouseOver = false;
    float preDragPosition = 0.0f;
    float preCurve = 4.0f; // 0 - 8
    float curve = 1.0f;

    void onHandleDrag (const Handle* h) override 
    { 
        if (h == &handleOne)
            routingChannel.setProperty (id::handleOne, h->getNormalizedPosition(), nullptr);
        else if (h == &handleTwo)
            routingChannel.setProperty (id::handleTwo, h->getNormalizedPosition(), nullptr);
        resized(); repaint(); 
    }
};
struct DraggableAssigner : public juce::DragAndDropContainer, 
                           public juce::Component
{
    DraggableAssigner (juce::ValueTree routingChannelBranch,
                       const juce::AudioProcessorValueTreeState& apvts)
      : routingChannel (routingChannelBranch)
    {

        auto paramID = routingChannel.getProperty (id::name).toString();
        if (paramID != "") name = apvts.getParameter (paramID)->getName (20);
    }
    void paint (juce::Graphics& g) override
    {
        juce::ignoreUnused (g);
       auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
       g.setColour (laf->getBackgroundDark());
       g.drawRect (getLocalBounds().toFloat(), 2.0f);
       
       g.setColour (juce::Colours::white);
       g.drawText (name, 
                   getLocalBounds().toFloat(), 
                   juce::Justification::centred); 
    }
    void mouseDrag (const juce::MouseEvent& event) override 
    {
        juce::ignoreUnused (event);
        startDragging ("MPE Source", this, juce::ScaledImage(), true);
    }
    void mouseUp (const juce::MouseEvent& event) override 
    {
        if(event.mods.isRightButtonDown())
        {
            juce::PopupMenu m;
            m.addItem(1, "Unassign");
            m.showMenuAsync (juce::PopupMenu::Options(), [&](int result)
            {
                // if(result == 1)
                // {
                //     mpeRouting.getChildWithName (mpeChannel)
                //               .getChildWithName (outputChannel)
                //               .setProperty (id::name, "", nullptr);
                //     name = "Drag to assign";
                //     repaint();
                // }  
                if(result == 1)
                {
                    routingChannel.setProperty (id::name, "", nullptr);
                    name = "Drag to assign";
                    repaint();
                }  
            });
        }
    }
    juce::ValueTree getMPEChannelRouting() { return routingChannel; }
    void setLabel (juce::String label) { name = label; repaint(); }
    void setState (juce::ValueTree routingBranch) { routingChannel = routingBranch; }
private:
    juce::ValueTree routingChannel;
    juce::String name {"Drag to assign"};
};

struct AssignableCurve : public juce::Component
{
    AssignableCurve (juce::ValueTree routingChannelBranch, 
                     const juce::AudioProcessorValueTreeState& vts)
      : assigner (routingChannelBranch, vts), 
        routingCurve (routingChannelBranch)
    {
        addAndMakeVisible (assigner);
        addAndMakeVisible (routingCurve);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        assigner.setBounds (b.removeFromTop (20));
        routingCurve.setBounds (b);
    }
private: 
    DraggableAssigner assigner;
    RoutingCurve routingCurve;
};
}