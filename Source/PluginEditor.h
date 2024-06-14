/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class TheVerbAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TheVerbAudioProcessorEditor (TheVerbAudioProcessor&);
    ~TheVerbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TheVerbAudioProcessor& audioProcessor;
    
    // Slider
    juce::Slider wet { juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow };
    juce::Label wetLabel;
    juce::Slider dry { juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow };
    juce::Label dryLabel;
    juce::Slider roomSize { juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow };
    juce::Label roomSizeLabel;
    juce::Slider decay { juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow };
    juce::Label decayLabel;
    juce::Slider modFreqMult { juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow };

    
    juce::Slider lpCutoff { juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow };
    juce::Label lpCutoffLabel;

    // Slider Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modFreqMultAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lpCutoffAttachment;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheVerbAudioProcessorEditor)
};
