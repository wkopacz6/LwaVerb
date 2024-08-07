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
    
    // Draw outer hexagon
    const auto outercenterX { innerHex->getX() + innerHex->getWidth() };
    const auto outercenterY { innerHex->getY() + innerHex->getHeight() };
    const auto rotationTransform { juce::AffineTransform::rotation(toAngle, outerHex->getX() + outerHex->getWidth() / 2,outerHex-> getY() + outerHex->getHeight() / 2) };
    outerHex->setTransform(rotationTransform);
    outerHex->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0);
    
    
    // Draw inner hexagon
    const auto ix { innerHex->getX() };
    const auto iy { innerHex->getY() };
    const auto iwidth { innerHex->getWidth() };
    const auto iheight { innerHex->getHeight()};
    const auto centerX { innerHex->getX() + innerHex->getWidth() / 2.0 };
    const auto centerY { innerHex->getY() + innerHex->getHeight() / 2.0 };
    
    
    const auto innerRotationTransform { juce::AffineTransform::rotation(toAngle, 1.63169, 1.4268105) };
    innerHex->setDrawableTransform(innerRotationTransform);
    
    const auto innerBounds { bounds.withSizeKeepingCentre(20, 20) };
    const auto containerBounds { bounds.withSizeKeepingCentre(40, 40) };
    auto innerImage { juce::Image(juce::Image::PixelFormat::RGB, containerBounds.getWidth(), containerBounds.getHeight(), true) };
    auto innerG { juce::Graphics(innerImage) };
    innerHex->drawWithin(innerG, juce::Rectangle<float>(10, 10, innerBounds.getWidth(), innerBounds.getHeight()), juce::RectanglePlacement::centred, 1.0);
    
    
    auto blur { melatonin::CachedBlur(static_cast<size_t>(11)) };
        
    g.drawImageAt(blur.render(innerImage), containerBounds.getX(), containerBounds.getY());
    // Blur inner hexagon
    
    // Draw endpoint dots
}
