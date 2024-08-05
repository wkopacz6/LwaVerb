
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TheVerbAudioProcessorEditor::TheVerbAudioProcessorEditor (TheVerbAudioProcessor& p)
    : AudioProcessorEditor (&p)
    , audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    addAndMakeVisible(wet);
    addAndMakeVisible(dry);
    addAndMakeVisible(decay);
    addAndMakeVisible(roomSize);
    
    wet.setLookAndFeel(&knobLnf);
}

TheVerbAudioProcessorEditor::~TheVerbAudioProcessorEditor()
{
}

//==============================================================================
void TheVerbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    dryAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, Params::dryId, dry);
    wetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, Params::wetId, wet);
    roomSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, Params::roomSizeId, roomSize);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, Params::decayId, decay);
#if USE_MODULATION
    modFreqMultAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, Params::modFreqId, modFreqMult);
#endif
    lpCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, Params::lpCutoffId, lpCutoff);
    
    dryLabel.setText("Dry", juce::dontSendNotification);
    wetLabel.setText("Wet", juce::dontSendNotification);
    roomSizeLabel.setText("Room Size", juce::dontSendNotification);
    decayLabel.setText("Decay Time", juce::dontSendNotification);
    lpCutoffLabel.setText("LP Cutoff", juce::dontSendNotification);
    
    addAndMakeVisible(dry);
    addAndMakeVisible(wet);
    addAndMakeVisible(roomSize);
    addAndMakeVisible(decay);
    addAndMakeVisible(dryLabel);
    addAndMakeVisible(wetLabel);
    addAndMakeVisible(roomSizeLabel);
    addAndMakeVisible(decayLabel);
#if DELAY_MOD
    addAndMakeVisible(modFreqMult);
#endif
    addAndMakeVisible(lpCutoff);
    addAndMakeVisible(lpCutoffLabel);
    
    wet.setRotaryParameters({ 0.0, 4.18879, true });
}

void TheVerbAudioProcessorEditor::resized()
{
    auto b { getLocalBounds() };
    
    // Margins
    b.removeFromTop(75);
    b.removeFromBottom(75);
    b.removeFromLeft(25);
    b.removeFromRight(25);
    
    auto sliderAndLabelBounds { b.removeFromLeft(70) };
    dry.setBounds(sliderAndLabelBounds.removeFromTop(120));
    dryLabel.setBounds(sliderAndLabelBounds);
    
    sliderAndLabelBounds = b.removeFromLeft(100);
    wet.setBounds(sliderAndLabelBounds.removeFromTop(120));
    wetLabel.setBounds(sliderAndLabelBounds);
    
    sliderAndLabelBounds = b.removeFromLeft(70);
    roomSize.setBounds(sliderAndLabelBounds.removeFromTop(120));
    roomSizeLabel.setBounds(sliderAndLabelBounds);
    
    sliderAndLabelBounds = b.removeFromLeft(70);
    decay.setBounds(sliderAndLabelBounds.removeFromTop(120));
    decayLabel.setBounds(sliderAndLabelBounds);

#if DELAY_MOD
    modFreqMult.setBounds(b.removeFromLeft(65));
#endif
    sliderAndLabelBounds = b.removeFromLeft(70);
    lpCutoff.setBounds(sliderAndLabelBounds.removeFromTop(120));
    lpCutoffLabel.setBounds(sliderAndLabelBounds);
}
