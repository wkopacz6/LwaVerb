
#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
static constexpr float logoSize { 40.0f };
}

//==============================================================================
TheVerbAudioProcessorEditor::TheVerbAudioProcessorEditor (TheVerbAudioProcessor& p)
    : AudioProcessorEditor (&p)
    , audioProcessor (p)
{
    setSize (700, 525);
    
    addAndMakeVisible(wet);
    addAndMakeVisible(dry);
    addAndMakeVisible(roomSize);
    addAndMakeVisible(decay);
    addAndMakeVisible(lpCutoff);
        
    dryAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, Params::dryId, dry.getSlider());
    wetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, Params::wetId, wet.getSlider());
    roomSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, Params::roomSizeId, roomSize.getSlider());
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, Params::decayId, decay.getSlider());
#if USE_MODULATION
    modFreqMultAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, Params::modFreqId, modFreqMult.getKnob());
#endif
    lpCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, Params::lpCutoffId, lpCutoff.getSlider());
}

TheVerbAudioProcessorEditor::~TheVerbAudioProcessorEditor()
{
}

//==============================================================================
void TheVerbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colors::backgroundTeal);
    
    auto b { getLocalBounds().toFloat() };
    
    // top margin
    b.removeFromTop(30.0f);
    
    logo->drawWithin(g, b.removeFromTop(logoSize), juce::RectanglePlacement::centred, 1.0);
    
    // below logo margin
    b.removeFromTop(25.0f);
}

void TheVerbAudioProcessorEditor::resized()
{
    auto b { getLocalBounds() };
    const auto margin { 25 };
    const auto knobSize { 200 };
    
    // Margins
    b.removeFromTop(margin);
    b.removeFromBottom(margin);
    b.removeFromLeft(margin);
    b.removeFromRight(margin);
    
    logo->setBounds(b.removeFromTop(logoSize));
    
    b.removeFromTop(margin);
    
    auto reverbControlsRow { b.removeFromTop(knobSize) };
    roomSize.setBounds(reverbControlsRow.removeFromLeft(knobSize));
    reverbControlsRow.removeFromLeft(margin);
    decay.setBounds(reverbControlsRow.removeFromLeft(knobSize));
    reverbControlsRow.removeFromLeft(margin);
    lpCutoff.setBounds(reverbControlsRow.removeFromLeft(knobSize));
    
    b.removeFromTop(margin);
    
    auto wetDryRow { b.removeFromTop(knobSize) };
    wetDryRow.removeFromLeft(112);
    wetDryRow.removeFromRight(112);
    dry.setBounds(wetDryRow.removeFromLeft(knobSize));
    wetDryRow.removeFromLeft(margin);
    wet.setBounds(wetDryRow.removeFromLeft(knobSize));
}
