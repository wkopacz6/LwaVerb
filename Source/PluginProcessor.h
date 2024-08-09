
#pragma once

#include "DspComponents.h"
#include "juce_audio_processors/juce_audio_processors.h"

#undef USE_MODULATION
#define USE_MODULATION 0

namespace Params
{
    static constexpr auto* dryId { "dry" };
    static constexpr auto* wetId { "wet" };
    static constexpr auto* roomSizeId { "roomSize" };
    static constexpr auto* decayId { "decay" };
    static constexpr auto* modFreqId { "modulationFreq" };
    static constexpr auto* modAmpId { "modulationAmp" };
    static constexpr auto* lpCutoffId { "lpCutoff" };
}

//==============================================================================
/**
*/
class TheVerbAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
    ,
                              public juce::AudioProcessorARAExtension
#endif
{
public:
    //==============================================================================
    TheVerbAudioProcessor();
    ~TheVerbAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    juce::AudioProcessorValueTreeState apvts;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    //==============================================================================
    Reverb<> reverbL;
    Reverb<> reverbR;
    std::array<float, NUM_CHANNELS> workingArray;
    std::array<float, NUM_CHANNELS> outputArray;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheVerbAudioProcessor)
};
