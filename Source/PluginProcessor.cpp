/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TheVerbAudioProcessor::TheVerbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
    #if !JucePlugin_IsMidiEffect
        #if !JucePlugin_IsSynth
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
        #endif
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
    #endif
            ),
      apvts (*this, nullptr, "ReverbState", createParameterLayout()),
      reverbL (35, 3),
      reverbR (35, 3)
#endif
{
}

TheVerbAudioProcessor::~TheVerbAudioProcessor()
{
}

//==============================================================================
const juce::String TheVerbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TheVerbAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool TheVerbAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool TheVerbAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double TheVerbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TheVerbAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int TheVerbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TheVerbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TheVerbAudioProcessor::getProgramName (int index)
{
    return {};
}

void TheVerbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TheVerbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    reverbL.configure (sampleRate);
    reverbL.setWet (0.75f);
    reverbL.setDry (0.25f);

    reverbR.configure (sampleRate);
    reverbR.setWet (0.75f);
    reverbR.setDry (0.25f);
}

void TheVerbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TheVerbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
    #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

            // This checks if the input layout matches the output layout
        #if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
        #endif

    return true;
    #endif
}
#endif

void TheVerbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Update reverb params
    const auto roomSizeFudge { (*apvts.getRawParameterValue (Params::roomSizeId) / 4.0f) + 75.0f };
    const auto rt60Fudge { (*apvts.getRawParameterValue (Params::decayId) / 2) + 3 };

    reverbL.setDry (*apvts.getRawParameterValue (Params::dryId));
    reverbL.setWet (*apvts.getRawParameterValue ((Params::wetId)));
    reverbL.setRoomSizeMs (roomSizeFudge);
    reverbL.setRt60 (rt60Fudge);
    reverbL.setLpCutoff (*apvts.getRawParameterValue (Params::lpCutoffId));
#if DELAY_MOD
    reverbL.setDelayModulation (*apvts.getRawParameterValue (Params::modFreqId), *apvts.getRawParameterValue (Params::modAmpId));
#endif

    reverbR.setDry (*apvts.getRawParameterValue (Params::dryId));
    reverbR.setWet (*apvts.getRawParameterValue ((Params::wetId)));
    reverbR.setRoomSizeMs (roomSizeFudge);
    reverbR.setRt60 (rt60Fudge);
    reverbR.setLpCutoff (*apvts.getRawParameterValue (Params::lpCutoffId));
#if DELAY_MOD
    reverbR.setDelayModulation (*apvts.getRawParameterValue (Params::modFreqId), *apvts.getRawParameterValue (Params::modAmpId));
#endif

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // fill each channel of the reverb, with the sample
        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            for (auto reverbChannel = 0; reverbChannel < NUM_CHANNELS; ++reverbChannel)
                workingArray[reverbChannel] = channelData[sample];

            if (channel == 0)
                outputArray = reverbL.process (workingArray);
            else
                outputArray = reverbR.process (workingArray);

            // Mix down to mono
            channelData[sample] = 0;
            for (auto data : outputArray)
                channelData[sample] += data;

            channelData[sample] = channelData[sample] / 8.0f;
        }
    }
}

//==============================================================================
bool TheVerbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TheVerbAudioProcessor::createEditor()
{
    return new TheVerbAudioProcessorEditor (*this);
}

//==============================================================================
void TheVerbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TheVerbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout TheVerbAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto dry = std::make_unique<juce::AudioParameterFloat> (Params::dryId, "Dry", 0.f, 1.0f, 0);
    params.push_back (std::move (dry));

    auto wet = std::make_unique<juce::AudioParameterFloat> (Params::wetId, "Wet", 0.f, 1.0f, 1.0f);
    params.push_back (std::move (wet));

    auto roomSize = std::make_unique<juce::AudioParameterFloat> (Params::roomSizeId, "Room Size", 25.f, 100.0f, 95.0f);
    params.push_back (std::move (roomSize));

    auto decay = std::make_unique<juce::AudioParameterFloat> (Params::decayId, "Decay", 0.f, 6.0f, 6.0f);
    params.push_back (std::move (decay));
#if DELAY_MOD
    auto modFreq = std::make_unique<juce::AudioParameterFloat> (Params::modFreqId, "Modulation Frequency", 0.f, 7.0f, 1.0f);
    params.push_back (std::move (modFreq));

    auto modAmp = std::make_unique<juce::AudioParameterFloat> (Params::modAmpId, "Modulation Amplitude", 0.f, 7000.0f, 500.0f);
    params.push_back (std::move (modAmp));
#endif
    auto lpCutoff = std::make_unique<juce::AudioParameterFloat> (Params::lpCutoffId, "Cutoff Frequency", 100.f, 18000.0f, 6000.0f);
    params.push_back (std::move (lpCutoff));

    return { params.begin(), params.end() };
}

//==============================================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheVerbAudioProcessor();
}
