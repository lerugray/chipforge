#include <Effects/TwoPoleResonantLowPass.h>

#include <gtest/gtest.h>

#include <cmath>
#include <juce_core/juce_core.h>

namespace
{
    constexpr int    kSampleRate  = 44100;
    constexpr int    kNumSamples  = 88200;  // 2.0s — stable for low-frequency tones
    constexpr int    kWarmup      = 20000;  // let IIR transients die down
    constexpr double kAmplitude   = 0.3;

    void fillSine (juce::AudioBuffer<float>& b, int numCh, int numSamples, double frequencyHz, double sampleRate)
    {
        b.setSize (numCh, numSamples, false, false, true);
        b.clear();
        if (numSamples <= 0)
            return;

        const double w = 2.0 * juce::MathConstants<double>::pi * (frequencyHz / sampleRate);
        for (int i = 0; i < numSamples; ++i)
        {
            const float s = (float) (kAmplitude * std::sin (w * (double) i));
            for (int ch = 0; ch < numCh; ++ch)
                b.setSample (ch, i, s);
        }
    }

    double rmsOfRange (const juce::AudioBuffer<float>& b, int channel, int start, int end)
    {
        jassert (end > start);
        jassert (channel < b.getNumChannels());
        long double s = 0.0L;
        const int n   = end - start;
        const float*  r = b.getReadPointer (channel);
        for (int i = start; i < end; ++i)
        {
            const long double v = (long double) r[i];
            s += v * v;
        }
        return (double) std::sqrt ((double) (s / (long double) n));
    }

    double dB (double ratio) noexcept
    {
        if (ratio <= 0.0) return -300.0;
        return 20.0 * std::log10 (ratio);
    }
} // namespace

TEST (TwoPoleResonantLowPass, RmsTiersAgainstReferenceCutoff)
{
    juce::AudioBuffer<float> a (1, kNumSamples);
    juce::AudioBuffer<float> b (1, kNumSamples);

    auto makeFilter = [] {
        TwoPoleResonantLowPass f { (double) kSampleRate };
        f.set_cutoff (500.0);
        f.set_resonance (0.7f);
        return f;
    };

    // 100 Hz — in-band (well below 500 Hz cutoff; expect ~0 dB within loose band)
    {
        TwoPoleResonantLowPass lp = makeFilter();
        fillSine (a, 1, kNumSamples, 100.0, (double) kSampleRate);
        b.makeCopyOf (a);
        lp.processBlock (b);
        const double in  = rmsOfRange (a, 0, kWarmup, kNumSamples);
        const double out = rmsOfRange (b, 0, kWarmup, kNumSamples);
        const double d   = dB (out / in);
        EXPECT_GT (d, -1.0) << "100 Hz passband should be near unity (got dB: " << d << ")";
        EXPECT_LT (d, 1.0) << "100 Hz passband should be near unity (got dB: " << d << ")";
    }

    // 5 kHz — should be well attenuated above cutoff
    {
        TwoPoleResonantLowPass lp = makeFilter();
        fillSine (a, 1, kNumSamples, 5000.0, (double) kSampleRate);
        b.makeCopyOf (a);
        lp.processBlock (b);
        const double in  = rmsOfRange (a, 0, kWarmup, kNumSamples);
        const double out = rmsOfRange (b, 0, kWarmup, kNumSamples);
        const double att  = dB (in) - dB (out);
        EXPECT_GT (att, 12.0) << "5 kHz should be strongly attenuated above 500 Hz cutoff (dB: " << att << ")";
    }

    // 1 kHz — above cutoff, 2nd-order (12 dB/oct): expect meaningful loss (wide band)
    {
        TwoPoleResonantLowPass lp = makeFilter();
        fillSine (a, 1, kNumSamples, 1000.0, (double) kSampleRate);
        b.makeCopyOf (a);
        lp.processBlock (b);
        const double in  = rmsOfRange (a, 0, kWarmup, kNumSamples);
        const double out = rmsOfRange (b, 0, kWarmup, kNumSamples);
        const double att  = dB (in) - dB (out);
        EXPECT_GT (att, 2.0) << "1 kHz should lose energy vs input (dB: " << att << ")";
        EXPECT_LT (att, 22.0) << "1 kHz should not be brick-walled by this 2nd-order LPF in test band (dB: " << att << ")";
    }
}
