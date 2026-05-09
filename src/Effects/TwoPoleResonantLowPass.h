#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>

//==============================================================================
// TwoPoleResonantLowPass
//
// 2nd-order (biquad) resonant low-pass, bilinear-transformed via JUCE IIR.
// One juce::dsp::IIR::Filter<float> per channel; deterministic, no random state.
//==============================================================================
class TwoPoleResonantLowPass
{
public:
    explicit TwoPoleResonantLowPass (double sample_rate);
    ~TwoPoleResonantLowPass() = default;

    void set_cutoff (double freqHz) noexcept;
    void set_resonance (float q) noexcept;

    void processBlock (juce::AudioBuffer<float>& buffer) noexcept;

    double getSampleRate() const noexcept { return sampleRate; }
    double getCutoffHz() const noexcept { return cutoffHz; }
    float  getQ() const noexcept { return q; }

private:
    void ensureChannelFilters (int numChannels) noexcept;
    void applyCurrentCoefficientsTo (size_t index) noexcept;

    double sampleRate;
    double cutoffHz;
    float  q;
    std::vector<juce::dsp::IIR::Filter<float>> perChannel;
};
