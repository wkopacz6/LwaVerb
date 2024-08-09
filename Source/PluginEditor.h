#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

#include "PluginProcessor.h"
#include "TheVerbKnob.h"
#include "UiHelpers.h"

//==============================================================================
/**
*/
class TheVerbAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TheVerbAudioProcessorEditor (TheVerbAudioProcessor&);
    ~TheVerbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    TheVerbAudioProcessor& audioProcessor;

    // LnF
    HexKnobLnf knobLnf;

    // Slider
    HexKnob wet { "WET" };
    HexKnob dry { "DRY" };
    HexKnob roomSize { "SIZE" };
    HexKnob decay { "DECAY" };
    HexKnob lpCutoff { "DAMPING" };

#if USE_MODULATION
    HexKnob modFreqMult { "MODULATION" };
#endif

    // Slider Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lpCutoffAttachment;

#if USE_MODULATION
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modFreqMultAttachment;
#endif

    std::unique_ptr<juce::Drawable> logo { juce::Drawable::createFromImageData (BinaryData::logo_svg, BinaryData::logo_svgSize) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheVerbAudioProcessorEditor)
};
