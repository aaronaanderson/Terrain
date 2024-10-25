#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

static const juce::Colour accent = juce::Colour::fromString ("#FFff5c00").darker (0.2f);
static const juce::Colour base = juce::Colour::fromString ("#FF00a3ff").darker (1.0f);
static const juce::Colour background = base.darker (0.5f);

class TerrainLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TerrainLookAndFeel()
      : juce::LookAndFeel_V4(juce::LookAndFeel_V4::ColourScheme(
            background, // windowBackground = 0,
            base, // widgetBackground,
            base, // menuBackground,
            juce::Colours::black, // outline,
            juce::Colours::white, // defaultText,
            background, // defaultFill,
            juce::Colours::white, // highlightedText,
            base, // highlightedFill,
            juce::Colours::white // menuText,
        ))
    {
        setColour (juce::Slider::thumbColourId, accent);
        setColour (juce::ToggleButton::ColourIds::tickColourId, accent);
        setDefaultSansSerifTypeface (getCustomFont());
    }
    static const juce::Colour getAccentColour() { return accent; }
    static const juce::Colour getBaseColour() { return base; }
    static const juce::Colour getBackgroundColour() { return background; }
    static const juce::Typeface::Ptr getCustomFont()
    {
        static auto typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::Paulle1V_ttf, 
                                                                       BinaryData::Paulle1V_ttfSize);
        return typeface;
    }
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos,
                                           float minSliderPos,
                                           float maxSliderPos,
                                           const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (slider.isBar())
        {
            g.setColour (slider.findColour (juce::Slider::trackColourId));
            g.fillRect (slider.isHorizontal() ? juce::Rectangle<float> (static_cast<float> (x), (float) y + 0.5f, sliderPos - (float) x, (float) height - 1.0f)
                                              : juce::Rectangle<float> ((float) x + 0.5f, sliderPos, (float) width - 1.0f, (float) y + ((float) height - sliderPos)));
    
            drawLinearSliderOutline (g, x, y, width, height, style, slider);
        }
        else
        {
            auto isTwoVal   = (style == juce::Slider::SliderStyle::TwoValueVertical   || style == juce::Slider::SliderStyle::TwoValueHorizontal);
            auto isThreeVal = (style == juce::Slider::SliderStyle::ThreeValueVertical || style == juce::Slider::SliderStyle::ThreeValueHorizontal);
    
            // auto trackWidth = juce::jmin (6.0f, slider.isHorizontal() ? (float) height * 0.25f : (float) width * 0.25f);
            auto trackWidth = static_cast<float> (getSliderThumbRadius (slider));
            juce::Point<float> startPoint (slider.isHorizontal() ? (float) x : (float) x + (float) width * 0.5f,
                                     slider.isHorizontal() ? (float) y + (float) height * 0.5f : (float) (height + y));
    
            juce::Point<float> endPoint (slider.isHorizontal() ? (float) (width + x) : startPoint.x,
                                   slider.isHorizontal() ? startPoint.y : (float) y);
    
            juce::Path backgroundTrack;
            backgroundTrack.startNewSubPath (startPoint);
            backgroundTrack.lineTo (endPoint);
            g.setColour (slider.findColour (juce::Slider::backgroundColourId));
            g.strokePath (backgroundTrack, { trackWidth, juce::PathStrokeType::mitered, juce::PathStrokeType::square });
    
            juce::Path valueTrack;
            juce::Point<float> minPoint, maxPoint, thumbPoint;
    
            if (isTwoVal || isThreeVal)
            {
                minPoint = { slider.isHorizontal() ? minSliderPos : (float) width * 0.5f,
                             slider.isHorizontal() ? (float) height * 0.5f : minSliderPos };
    
                if (isThreeVal)
                    thumbPoint = { slider.isHorizontal() ? sliderPos : (float) width * 0.5f,
                                   slider.isHorizontal() ? (float) height * 0.5f : sliderPos };
    
                maxPoint = { slider.isHorizontal() ? maxSliderPos : (float) width * 0.5f,
                             slider.isHorizontal() ? (float) height * 0.5f : maxSliderPos };
            }
            else
            {
                auto kx = slider.isHorizontal() ? sliderPos : ((float) x + (float) width * 0.5f);
                auto ky = slider.isHorizontal() ? ((float) y + (float) height * 0.5f) : sliderPos;
    
                minPoint = startPoint;
                maxPoint = { kx, ky };
            }
    
            auto thumbWidth = getSliderThumbRadius (slider);
    
            valueTrack.startNewSubPath (minPoint);
            valueTrack.lineTo (isThreeVal ? thumbPoint : maxPoint);
            g.setColour (slider.findColour (juce::Slider::trackColourId));
            g.strokePath (valueTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
    
            if (! isTwoVal)
            {
                g.setColour (slider.findColour (juce::Slider::thumbColourId));
                g.fillRect (juce::Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre (isThreeVal ? thumbPoint : maxPoint));
            }
    
            if (isTwoVal || isThreeVal)
            {
                auto sr = juce::jmin (trackWidth, (slider.isHorizontal() ? (float) height : (float) width) * 0.4f);
                auto pointerColour = slider.findColour (juce::Slider::thumbColourId);
    
                if (slider.isHorizontal())
                {
                    drawPointer (g, minSliderPos - sr,
                                 juce::jmax (0.0f, (float) y + (float) height * 0.5f - trackWidth * 2.0f),
                                 trackWidth * 2.0f, pointerColour, 2);
    
                    drawPointer (g, maxSliderPos - trackWidth,
                                 juce::jmin ((float) (y + height) - trackWidth * 2.0f, (float) y + (float) height * 0.5f),
                                 trackWidth * 2.0f, pointerColour, 4);
                }
                else
                {
                    drawPointer (g, juce::jmax (0.0f, (float) x + (float) width * 0.5f - trackWidth * 2.0f),
                                 minSliderPos - trackWidth,
                                 trackWidth * 2.0f, pointerColour, 1);
    
                    drawPointer (g, juce::jmin ((float) (x + width) - trackWidth * 2.0f, (float) x + (float) width * 0.5f), maxSliderPos - sr,
                                 trackWidth * 2.0f, pointerColour, 3);
                }
            }
    
            if (slider.isBar())
                drawLinearSliderOutline (g, x, y, width, height, style, slider);
        }
    }
    
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto outline = slider.findColour (juce::Slider::rotarySliderOutlineColourId);
        auto fill    = slider.findColour (juce::Slider::rotarySliderFillColourId);
    
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (10);
    
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = juce::jmin (10.0f, radius * 0.5f);
        auto arcRadius = radius - lineW * 0.5f;
    
        juce::Path backgroundArc;
        backgroundArc.addCentredArc (bounds.getCentreX(),
                                     bounds.getCentreY(),
                                     arcRadius,
                                     arcRadius,
                                     0.0f,
                                     rotaryStartAngle,
                                     rotaryEndAngle,
                                     true);
    
        g.setColour (outline);
        g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::mitered, juce::PathStrokeType::square));
    
        if (slider.isEnabled())
        {
            juce::Path valueArc;
            valueArc.addCentredArc (bounds.getCentreX(),
                                    bounds.getCentreY(),
                                    arcRadius,
                                    arcRadius,
                                    0.0f,
                                    rotaryStartAngle,
                                    toAngle,
                                    true);
    
            g.setColour (fill);
            g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::mitered, juce::PathStrokeType::square));
        }
    
        //auto thumbWidth = lineW;// * 2.0f;
        juce::Point<float> thumbPoint (bounds.getCentreX() + arcRadius * std::cos (toAngle - juce::MathConstants<float>::halfPi),
                                      bounds.getCentreY() + arcRadius * std::sin (toAngle - juce::MathConstants<float>::halfPi));
    
        g.setColour (slider.findColour (juce::Slider::thumbColourId));
        // g.fillEllipse (juce::Rectangle<float> (thumbWidth, thumbWidth).withCentre (thumbPoint));
        juce::Path thumbArc;
        const float halfThumbArcLength = juce::MathConstants<float>::halfPi * 0.15f;
        thumbArc.addCentredArc (bounds.getCentreX(), 
                                bounds.getCentreY(), 
                                arcRadius, 
                                arcRadius, 
                                0.0f, 
                                toAngle - halfThumbArcLength, 
                                toAngle + halfThumbArcLength, 
                                true);
        g.strokePath (thumbArc, juce::PathStrokeType (lineW, juce::PathStrokeType::mitered, juce::PathStrokeType::butt));
    }   
    
    void drawTickBox (juce::Graphics& g, juce::Component& component,
                      float x, float y, float w, float h,
                      const bool ticked,
                      [[maybe_unused]] const bool isEnabled,
                      [[maybe_unused]] const bool shouldDrawButtonAsHighlighted,
                      [[maybe_unused]] const bool shouldDrawButtonAsDown) override
    {
        juce::Rectangle<float> tickBounds (x, y, w, h);
    
        g.setColour (component.findColour (juce::ToggleButton::tickDisabledColourId));
        g.drawRoundedRectangle (tickBounds, 4.0f, 1.0f);
    
        if (ticked)
        {
            g.setColour (component.findColour (juce::ToggleButton::tickColourId));
            // auto tick = getTickShape (0.75f);
            // g.fillPath (tick, tick.getTransformToScaleToFit (tickBounds.reduced (4, 5).toFloat(), false));
            // g.fillRect (tickBounds);
            // g.drawRoundedRectangle (tickBounds, 1.0f, 1.0f);
            g.fillRoundedRectangle (tickBounds.reduced (2.0f), 4.0f);
        }
    }
};