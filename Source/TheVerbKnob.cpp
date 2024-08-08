/*
  ==============================================================================

    TheVerbKnob.cpp
    Created: 5 Aug 2024 10:14:06am
    Author:  Walter Kopacz

  ==============================================================================
*/

#include "TheVerbKnob.h"

void HexKnobLnf::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                      const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    // We need the bounds to be square or else the knob might get cutoff based on orientation
    const auto size { juce::jmin(width, height) };
    auto preadjustedBounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    auto bounds = preadjustedBounds.withSizeKeepingCentre(size, size);
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    // Draw endpoint dots
    
    // A little bit of geometry to make the dots the right height
    const auto dotOffsetMultiplier { std::sin(30.0 * 2.0 * 3.14159 / 360.0) / (2 * std::sin(30.0 * 2.0 * 3.14159 / 360.0) + 1) };

    const auto bottomHexHeight { bounds.getHeight() * dotOffsetMultiplier };
    auto boundsCopy { bounds };
    auto bottomHexBounds { boundsCopy.removeFromBottom(bottomHexHeight) };
    const auto dotSize { bounds.proportionOfWidth(0.0365) };
    
    g.setColour(Colors::hexKnobLightGray);
    auto leftEndMarkerBounds { bottomHexBounds.removeFromLeft(dotSize) };
    g.fillEllipse(leftEndMarkerBounds.removeFromTop(dotSize));
    
    auto rightEndMarkerBounds { bottomHexBounds.removeFromRight(dotSize) };
    g.fillEllipse(rightEndMarkerBounds.removeFromTop(dotSize));
    
    // Draw outer hexagon
    const auto outerHexBounds { outerHex->getDrawableBounds() };
    const auto outerRotationTransform { juce::AffineTransform::rotation(toAngle, outerHexBounds.getCentreX(), outerHexBounds.getCentreY()) };
    outerHex->setTransform(outerRotationTransform);
    outerHex->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0);
    
    // Draw inner hexagon
    const auto innerHexBounds { innerHex->getDrawableBounds() };
    const auto innerRotationTransform { juce::AffineTransform::rotation(toAngle, innerHexBounds.getCentreX(), innerHexBounds.getCentreY()) };
    innerHex->setTransform(innerRotationTransform);
    
    const auto targetInnerHexBounds { bounds.withSizeKeepingCentre(bounds.proportionOfWidth(0.25), bounds.proportionOfHeight(0.25)) };
    // We want the bounds for the actual image that contains the inner hex to be bigger than the hex so it won't cutoff the blur
    const auto imageBounds { bounds.withSizeKeepingCentre(bounds.proportionOfWidth(0.5), bounds.proportionOfHeight(0.5)) };
    auto innerImage { juce::Image(juce::Image::PixelFormat::RGB, imageBounds.getWidth(), imageBounds.getHeight(), true) };
    auto innerG { juce::Graphics(innerImage) };
    innerHex->drawWithin(innerG, juce::Rectangle<float>(targetInnerHexBounds.getWidth() / 2, targetInnerHexBounds.getHeight() / 2, targetInnerHexBounds.getWidth(), targetInnerHexBounds.getHeight()), juce::RectanglePlacement::centred, 1.0);
            
    // Blur the inner hex
    if (const auto blurRadius { static_cast<size_t>(sliderPos * bounds.proportionOfWidth(0.1)) }; blurRadius > 0)
    {
        melatonin::CachedBlur blur { blurRadius };
        innerImage = blur.render(innerImage);
    }

    g.drawImageAt(innerImage, imageBounds.getX(), imageBounds.getY());
}

//==============================================================================

HexKnob::HexKnob(juce::StringRef name)
{
    knob.setLookAndFeel(&lnf);
    // set the start and end angles to the right values
    knob.setRotaryParameters({ 0.0, 4.18879, true });
    
    label.setText(name, juce::NotificationType::dontSendNotification);
    label.setColour(juce::Label::ColourIds::textColourId, Colors::hexKnobLightGray);
    label.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(knob);
    addAndMakeVisible(label);
}

void HexKnob::resized()
{
    auto b { getLocalBounds() };
    const auto labelSize { 15 };
    const auto margin { 10 };
    
    label.setBounds(b.removeFromBottom(labelSize));
    b.removeFromBottom(margin);
    knob.setBounds(b);
}
