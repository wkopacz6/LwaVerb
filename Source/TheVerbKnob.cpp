/*
  ==============================================================================

    TheVerbKnob.cpp
    Created: 5 Aug 2024 10:14:06am
    Author:  Walter Kopacz

  ==============================================================================
*/

#include "TheVerbKnob.h"

void TheVerbKnobLnF::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                      const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (10);
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    // Draw endpoint dots
    
    // A little bit of geometry to make the dots the right height
    const auto dotOffsetMultiplier { std::sin(30.0 * 2.0 * 3.14159 / 360.0) / (2 * std::sin(30.0 * 2.0 * 3.14159 / 360.0) + 1) };

    
    const auto bottomHexHeight { bounds.getHeight() * dotOffsetMultiplier };
    auto boundsCopy { bounds };
    auto bottomHexBounds { boundsCopy.removeFromBottom(bottomHexHeight) };
    const auto dotSize { bounds.proportionOfWidth(0.0365) };
    
    g.setColour(juce::Colour(0xff6f6f6f));
    auto leftEndMarkerBounds { bottomHexBounds.removeFromLeft(dotSize) };
    g.fillEllipse(leftEndMarkerBounds.removeFromTop(dotSize));
    
    auto rightEndMarkerBounds { bottomHexBounds.removeFromRight(dotSize) };
    g.fillEllipse(rightEndMarkerBounds.removeFromTop(dotSize));
    
    // Draw outer hexagon
    const auto outerHexBounds { outerHex->getDrawableBounds() };
    const auto outerRotationTransform { juce::AffineTransform::rotation(toAngle, outerHexBounds.getCentreX(), outerHexBounds.getCentreY()) };
    outerHex->setTransform(outerRotationTransform);
    outerHex->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0);
    
    const auto innerHexBounds { innerHex->getDrawableBounds() };
    const auto innerRotationTransform { juce::AffineTransform::rotation(toAngle, innerHexBounds.getCentreX(), innerHexBounds.getCentreY()) };
    innerHex->setTransform(innerRotationTransform);
    
    const auto targetInnerHexBounds { bounds.withSizeKeepingCentre(bounds.proportionOfWidth(0.25), bounds.proportionOfHeight(0.25)) };
    // We want the bounds for the actual image that contains the inner hex to be bigger than the hex so it won't cutoff the blur
    const auto imageBounds { bounds.withSizeKeepingCentre(bounds.proportionOfWidth(0.5), bounds.proportionOfHeight(0.5)) };
    auto innerImage { juce::Image(juce::Image::PixelFormat::RGB, imageBounds.getWidth(), imageBounds.getHeight(), true) };
    auto innerG { juce::Graphics(innerImage) };
    innerHex->drawWithin(innerG, juce::Rectangle<float>(targetInnerHexBounds.getWidth() / 2, targetInnerHexBounds.getHeight() / 2, targetInnerHexBounds.getWidth(), targetInnerHexBounds.getHeight()), juce::RectanglePlacement::centred, 1.0);
            
    
    const auto blurRadius { static_cast<size_t>(sliderPos * bounds.proportionOfWidth(0.1)) };
    
    if (blurRadius > 0)
    {
        melatonin::CachedBlur blur { blurRadius };
        innerImage = blur.render(innerImage);
    }

    // Blur the inner hex
    g.drawImageAt(innerImage, imageBounds.getX(), imageBounds.getY());
}
