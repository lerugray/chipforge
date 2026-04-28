#include "Effects/TwoPoleResonantLowPass.h"

#include <juce_core/juce_core.h>

static double clampToNyquist (double rate, double f) noexcept
{
    const double limit = 0.49 * rate;
    if (f < 1.0)  return 1.0;
    if (f > limit) return limit;
    return f;
}

static float clampQ (float v) noexcept
{
    if (v < 0.05f)  return 0.05f;
    if (v > 20.0f)  return 20.0f;
    return v;
}

static juce::dsp::ProcessSpec makeMonoSpec (double sampleRate) noexcept
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 1;
    spec.numChannels = 1;
    return spec;
}

TwoPoleResonantLowPass::TwoPoleResonantLowPass (double sample_rate)
    : sampleRate (juce::jmax (1.0, sample_rate))
    , cutoffHz (clampToNyquist (this->sampleRate, 1000.0))
    , q (0.707f)
{
    jassert (juce::isfinite (this->sampleRate) && this->sampleRate > 0.0);
}

void TwoPoleResonantLowPass::set_cutoff (double freqHz) noexcept
{
    cutoffHz = clampToNyquist (sampleRate, freqHz);
    for (size_t i = 0; i < perChannel.size(); ++i)
        applyCurrentCoefficientsTo (i);
}

void TwoPoleResonantLowPass::set_resonance (float juceQ) noexcept
{
    q = clampQ (juceQ);
    for (size_t i = 0; i < perChannel.size(); ++i)
        applyCurrentCoefficientsTo (i);
}

void TwoPoleResonantLowPass::applyCurrentCoefficientsTo (size_t index) noexcept
{
    jassert (index < perChannel.size());
    auto& flt = perChannel[index];
    flt.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, static_cast<float> (cutoffHz), q);
    flt.reset();
}

void TwoPoleResonantLowPass::ensureChannelFilters (int numChannels) noexcept
{
    if (numChannels <= 0)
        return;

    const size_t n = (size_t) numChannels;
    if (perChannel.size() == n)
        return;

    perChannel.resize (n, juce::dsp::IIR::Filter<float>());
    const auto spec = makeMonoSpec (sampleRate);

    for (size_t i = 0; i < n; ++i)
    {
        applyCurrentCoefficientsTo (i);
        perChannel[i].prepare (spec);
    }
}

void TwoPoleResonantLowPass::processBlock (juce::AudioBuffer<float>& buffer) noexcept
{
    if (buffer.getNumSamples() <= 0)
        return;

    const int numCh = buffer.getNumChannels();
    ensureChannelFilters (numCh);

    const int n = buffer.getNumSamples();
    for (int ch = 0; ch < numCh; ++ch)
    {
        float* w = buffer.getWritePointer (ch);
        for (int i = 0; i < n; ++i)
            w[i] = perChannel[(size_t) ch].processSample (w[i]);
        perChannel[(size_t) ch].snapToZero();
    }
}
