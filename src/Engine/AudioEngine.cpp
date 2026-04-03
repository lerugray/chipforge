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

void AudioEngine::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    nesSynth.prepare(sampleRate, samplesPerBlockExpected);
}

void AudioEngine::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Audio thread — no allocation, no locking, no I/O.
    nesSynth.renderBlock(bufferToFill);
}

void AudioEngine::releaseResources()
{
    // NES synth holds no external resources — nothing to release.
}
