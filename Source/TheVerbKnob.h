/*
  ==============================================================================

    TheVerbKnob.h
    Created: 5 Aug 2024 10:14:06am
    Author:  Walter Kopacz

  ==============================================================================
*/

#pragma once

#include "juce_gui_basics/juce_gui_basics.h"
#include "BinaryData.h"
#include "melatonin_blur/melatonin_blur.h"

#include "UiHelpers.h"

class HexKnobLnf : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
private:
    std::unique_ptr<juce::Drawable> outerHex { juce::Drawable::createFromImageData(BinaryData::outer_hex_svg, BinaryData::outer_hex_svgSize) };
    std::unique_ptr<juce::Drawable> innerHex { juce::Drawable::createFromImageData(BinaryData::inner_hex_svg, BinaryData::inner_hex_svgSize) };
};

//==============================================================================

class HexKnob : public juce::Component
{
public:
    HexKnob(juce::StringRef name);
    
    void resized() override;
    juce::Slider& getSlider() { return knob; }
private:
    juce::Slider knob { juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox };
    juce::Label label;
    HexKnobLnf lnf;
};
