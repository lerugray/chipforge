#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include "Synths/NES2A03/NES2A03Synth.h"

class NES2A03SynthTests final : public juce::UnitTest
{
public:
    NES2A03SynthTests()
        : juce::UnitTest("NES2A03Synth", "Synths")
    {
    }

    void runTest() override
    {
        beginTest("note commands are applied during render");
        NES2A03Synth synth;
        synth.prepare(48000.0, 256);

        juce::AudioBuffer<float> buffer(2, 64);
        juce::AudioSourceChannelInfo info(&buffer, 0, buffer.getNumSamples());

        synth.noteOn(60);
        synth.renderBlock(info);
        expectEquals(synth.getActiveNote(), 60);

        synth.noteOff();
        synth.renderBlock(info);
        expectEquals(synth.getActiveNote(), -1);

        beginTest("oversized render requests clear safely");
        NES2A03Synth guardedSynth;
        guardedSynth.prepare(48000.0, 16);

        juce::AudioBuffer<float> oversizedBuffer(2, 1200);
        oversizedBuffer.clear();
        oversizedBuffer.applyGain(0.0f);
        for (int channel = 0; channel < oversizedBuffer.getNumChannels(); ++channel)
            oversizedBuffer.setSample(channel, 0, 1.0f);

        juce::AudioSourceChannelInfo oversizedInfo(&oversizedBuffer,
                                                   0,
                                                   oversizedBuffer.getNumSamples());
        guardedSynth.noteOn(60);
        guardedSynth.renderBlock(oversizedInfo);

        expectEquals(guardedSynth.getActiveNote(), 60);
        expectEquals(oversizedBuffer.getMagnitude(0, oversizedBuffer.getNumSamples()), 0.0f);
    }
};

static NES2A03SynthTests nes2A03SynthTests;
