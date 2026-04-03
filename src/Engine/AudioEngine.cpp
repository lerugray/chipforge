#include "AudioEngine.h"

AudioEngine::AudioEngine(TransportState& transport)
    : transportState(transport)
{
    setAudioChannels(0, 2);  // no inputs, stereo output
}

AudioEngine::~AudioEngine()
{
    shutdownAudio();
}

void AudioEngine::prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate)
{
    currentSampleRate = sampleRate;
    currentAngle      = 0.0;
    updateAngleDelta();
}

void AudioEngine::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Audio thread — no allocation, no locking, no I/O.
    if (!transportState.getIsPlaying())
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    auto* leftChannel  = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightChannel = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

    for (int i = 0; i < bufferToFill.numSamples; ++i)
    {
        const float sample = testToneAmplitude * (float) std::sin(currentAngle);

        leftChannel[i]  = sample;
        rightChannel[i] = sample;

        currentAngle += angleDelta;
        if (currentAngle >= juce::MathConstants<double>::twoPi)
            currentAngle -= juce::MathConstants<double>::twoPi;
    }
}

void AudioEngine::releaseResources()
{
    // Nothing to release in Phase 0.
}

void AudioEngine::updateAngleDelta()
{
    angleDelta = (testToneFrequency / currentSampleRate) * juce::MathConstants<double>::twoPi;
}
