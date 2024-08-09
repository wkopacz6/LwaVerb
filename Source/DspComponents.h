#pragma once

#include <cmath>
#include <math.h>

#include "../dsp/delay.h"
#include "juce_dsp/juce_dsp.h"

#undef DELAY_MOD
#define DELAY_MOD 0

#undef NUM_CHANNELS
#define NUM_CHANNELS 8
#undef DIFF_STEPS
#define DIFF_STEPS 6

// NOTE: This mixer code was lifted from signalsmith code
namespace Mixer
{
    // Use `Householder<float, 8>::inPlace(data)` - size must be ≥ 1
    template <typename Sample, int size>
    class Householder
    {
        static constexpr Sample multiplier { -2.0 / size };

    public:
        static void inPlace (Sample* arr)
        {
            float sum = 0;
            for (int i = 0; i < size; ++i)
            {
                sum += arr[i];
            }

            sum *= multiplier;

            for (int i = 0; i < size; ++i)
            {
                arr[i] += sum;
            }
        };
    };

    // Use like `Hadamard<float, 8>::inPlace(data)` - size must be a power of 2
    template <typename Sample, int size>
    class Hadamard
    {
    public:
        static inline void recursiveUnscaled (Sample* data)
        {
            if (size <= 1)
                return;
            constexpr int hSize = size / 2;

            // Two (unscaled) Hadamards of half the size
            Hadamard<Sample, hSize>::recursiveUnscaled (data);
            Hadamard<Sample, hSize>::recursiveUnscaled (data + hSize);

            // Combine the two halves using sum/difference
            for (int i = 0; i < hSize; ++i)
            {
                float a = data[i];
                float b = data[i + hSize];
                data[i] = (a + b);
                data[i + hSize] = (a - b);
            }
        }

        static inline void inPlace (Sample* data)
        {
            recursiveUnscaled (data);

            Sample scalingFactor = std::sqrt (1.0 / size);
            for (int c = 0; c < size; ++c)
            {
                data[c] *= scalingFactor;
            }
        }
    };
}

using Delay = signalsmith::delay::Delay<float, signalsmith::delay::InterpolatorKaiserSinc4>;

class SinglePoleLowPass
{
public:
    SinglePoleLowPass()
    {
        calcCutoff (2000.0f);
    }

    void calcCutoff (float freq)
    {
        const auto coeffs { juce::IIRCoefficients::makeLowPass (sampleRate, freq, 0.7071) };
        juceFilter.setCoefficients (coeffs);
    }

    float process (float sample)
    {
        return juceFilter.processSingleSampleRaw (sample);
    }
    float sampleRate { 44100 };

private:
    juce::IIRFilter juceFilter;

    float a0;
    float b0;
    float output;
};

class TriangleModulator
{
public:
    TriangleModulator()
    {
    }

    void configure (int theSampleRate)
    {
        sampleRate = theSampleRate;
        setFrequency (1.0f);
    }

    /**
     @returns the number of samples to delay based on modulating by a triangle wave
     */
    int getModulatorValue()
    {
        const auto period { static_cast<int> (1.0f / modulationFrequencyInSamples) };
        // this doesn't actually give you the period your looking for - use this as triangle wave instead y = (A/P) * (P - abs(x % (2*P) - P) )
        return (amplitude / period) * (period - (std::abs ((currentSampleIdx++ % (2 * period)) - period)));
    }

    void setFrequency (float freqInHz)
    {
        modulationFrequencyInSamples = freqInHz / sampleRate;
    }

    void setAmplitude (int amp) { amplitude = amp; }

private:
    // Only using whole number freqs for simplicity
    float modulationFrequencyInSamples { 2.0f };
    int amplitude { 10 };
    float sampleRate { 44100.0f };
    int currentSampleIdx { 0 };
};

template <int channels = NUM_CHANNELS>
class MultiMixedFeedback
{
public:
    void setDelayMs (float delay)
    {
        delayMs = delay;
    }

    void setDecayGain (float gain)
    {
        if (decayGain < 1.0f)
            decayGain = gain;
    }

    void setLpCutoff (float freq)
    {
        for (auto& filt : lowPassFilters)
            filt.calcCutoff (freq);
    }

    void setModulatorAmplitudes (int amp)
    {
        for (auto& modulator : modulators)
            modulator.setAmplitude (amp);
    }

    void setModulatorFrequencies (int freqInHz)
    {
        for (auto i = 0; i < channels; ++i)
            modulators[i].setFrequency (i + freqInHz);
    }

    /**
     @brief Setup the delay lines
     */
    void configure (float theSampleRate)
    {
        sampleRate = theSampleRate;
        delaySamplesBase = 100.0f * 0.001 * sampleRate;
        for (int i = 0; i < channels; ++i)
        {
            const float r = i * 1.5 / channels;
            numDelaySamples[i] = std::pow (2, r) * delaySamplesBase;
            delays[i].resize (numDelaySamples[i] + 1);
            delays[i].reset();
            lowPassFilters[i].sampleRate = sampleRate;
        }

        for (auto i = 0; i < channels; ++i)
            modulators[i].configure (sampleRate);

        setModulatorFrequencies (1);
        setModulatorAmplitudes (15);
    }

    std::array<float, channels> process (std::array<float, channels> input)
    {
        std::array<float, channels> delayed;
        // TODO: Your modulator amplitude is relatively very small. Need to increase to on the order of thousands of samples. But not exceed the size of the delay line!
        for (auto i = 0; i < channels; ++i)
        {
            const auto modValue { modulators[i].getModulatorValue() };
            const auto rawDelay { numDelaySamples[i] };
            if (modValue < (rawDelay - 100))
                delayed[i] = delays[i].read (rawDelay - modValue);
            else
                delayed[i] = rawDelay;
        }

        // Mix the delays
        std::array<float, channels> delayedAndMixed { delayed };
        Mixer::Householder<float, channels>::inPlace (delayedAndMixed.data());

        // Apply decay gain, add to input, and write back into delays
        for (auto i = 0; i < channels; ++i)
        {
            auto sum = input[i] + (decayGain * delayedAndMixed[i]);
            delays[i].write (lowPassFilters[i].process (sum));
        }

        return delayedAndMixed;
    }

private:
    float delayMs { 200.0f };
    float decayGain { 0.1f };
    float sampleRate { 44100.0f };

    std::array<int, channels> numDelaySamples;
    //#if USE_MODULATION
    std::array<TriangleModulator, channels> modulators;
    //#endif
    float modFreqMultiplier { 1.0f };
    std::array<Delay, channels> delays;
    float delaySamplesBase { 0.0f };

    // for lowpass
    std::array<SinglePoleLowPass, channels> lowPassFilters;
    float lpCutoff { 2000.0f };
};

template <int channels = NUM_CHANNELS>
class DiffusionStep
{
    using ChannelArray = std::array<float, channels>;

public:
    void setDelayMsRange (float delayRange)
    {
        delayMsRange = delayRange;
    }

    void configure (float theSampleRate)
    {
        sampleRate = theSampleRate;

        const auto delaySamplesRange { delayMsRange * 0.001 * sampleRate };
        for (auto i = 0; i < channels; ++i)
        {
            const auto rangeLow = delaySamplesRange * i / channels;
            const auto rangeHigh = delaySamplesRange * (i + 1) / channels;
            auto randomNumGenerator { juce::Random() };
            delaySamples[i] = randomNumGenerator.nextInt ({ static_cast<int> (rangeLow), static_cast<int> (rangeHigh) });
            delays[i].resize (delaySamples[i] + 1);
            delays[i].reset();
            flipPolarity[i] = randomNumGenerator.nextInt() % 2;
        }
    }

    ChannelArray process (ChannelArray input)
    {
        // Delay
        ChannelArray delayed;
        for (auto i = 0; i < channels; ++i)
        {
            delays[i].write (input[i]);
            delayed[i] = delays[i].read (delaySamples[i]);
        }

        // Mix with a Hadamard
        ChannelArray delayedAndMixed { delayed };
        Mixer::Hadamard<float, channels>::inPlace (delayedAndMixed.data());

        // Flip some polarities
        for (auto i = 0; i < channels; ++i)
            if (flipPolarity[i])
                delayedAndMixed[i] *= -1;

        return delayedAndMixed;
    }

private:
    float delayMsRange { 100.0f };
    float sampleRate { 44100.0f };

    std::array<int, channels> delaySamples;
    std::array<Delay, channels> delays;
    std::array<bool, channels> flipPolarity;
};

template <int channels = NUM_CHANNELS, int stepCount = DIFF_STEPS>
class HalfLengthChannelDiffuser
{
    using ChannelArray = std::array<float, channels>;

public:
    HalfLengthChannelDiffuser (float diffusionMs)
    {
        updateDiffusionMs (50.0f);
    }

    void configure (float sampleRate)
    {
        for (auto& step : steps)
            step.configure (sampleRate);
    }

    ChannelArray process (ChannelArray input)
    {
        for (auto& step : steps)
            input = step.process (input);

        return input;
    }

    void updateDiffusionMs (float diffusionMs)
    {
        for (auto& step : steps)
        {
            diffusionMs *= 0.5;
            step.setDelayMsRange (diffusionMs);
        }
    };

private:
    std::array<DiffusionStep<channels>, stepCount> steps;
};

template <int channels = NUM_CHANNELS, int diffusionSteps = DIFF_STEPS>
class Reverb
{
    using ChannelArray = std::array<float, channels>;

public:
    Reverb (float theRoomSizeMs, float theRt60, float dry = 0, float wet = 1)
        : diffuser (theRoomSizeMs)
    {
        updateParams();
    }

    void configure (float sampleRate)
    {
        feedback.configure (sampleRate);
        diffuser.configure (sampleRate);
        sampleRate = sampleRate;
    }

    ChannelArray process (ChannelArray input)
    {
        const ChannelArray diffuse { diffuser.process (input) };
        const ChannelArray reverbed { feedback.process (diffuse) };
        ChannelArray output;
        for (auto i = 0; i < channels; ++i)
            output[i] = dry * input[i] + wet * reverbed[i];

        return output;
    }

    void setWet (float wetAmount) { wet = wetAmount; }
    void setDry (float dryAmount) { dry = dryAmount; }

    void setRoomSizeMs (float size)
    {
        roomSizeMs = 101.0f - size;
        updateParams();
    }

    void setRt60 (float theRt60)
    {
        rt60 = theRt60;
        updateParams();
    }

    void setLpCutoff (float freq)
    {
        if (freq == lpFreq)
            return;

        feedback.setLpCutoff (freq);
        lpFreq = freq;
    }

    void setDelayModulation (int freqInHz, int amplitude)
    {
        feedback.setModulatorFrequencies (freqInHz);
        feedback.setModulatorAmplitudes (amplitude);
    }

private:
    MultiMixedFeedback<channels> feedback;
    HalfLengthChannelDiffuser<channels, diffusionSteps> diffuser;

    float wet { 1.0 };
    float dry { 0.0 };

    float roomSizeMs { 50.0f };
    float rt60 { 12.0f };
    float sampleRate { 48000 };
    float lpFreq { 4000.0f };

    void updateParams()
    {
        diffuser.updateDiffusionMs (roomSizeMs);

        feedback.setDelayMs (roomSizeMs);

        // How long does our signal take to go around the feedback loop?
        const auto typicalLoopMs { roomSizeMs * 2.5 };

        // How many times will it do that during our RT60 period?
        const auto loopsPerRt60 { (rt60 / 2) / (typicalLoopMs * 0.001) };

        // This tells us how many dB to reduce per loop
        const auto dbPerCycle = -60 / loopsPerRt60;

        feedback.setDecayGain (std::pow (10, dbPerCycle * 0.8f));
    }
};
