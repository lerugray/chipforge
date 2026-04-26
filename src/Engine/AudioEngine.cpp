#include "AudioEngine.h"

#include <algorithm>

namespace
{
Pattern createStartupPattern()
{
    Pattern pattern(16, 1);
    pattern.setCell(0, 0, PatternCell::makeNote(60));
    pattern.setCell(4, 0, PatternCell::makeNote(64));
    pattern.setCell(8, 0, PatternCell::makeNote(67));
    pattern.setCell(12, 0, PatternCell::makeNoteOff());
    return pattern;
}
}

AudioEngine::AudioEngine(TransportState& transport)
    : transportState(transport),
      trackerPattern(createStartupPattern()),
      trackerEngine(trackerPattern)
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
    trackerEngine.prepare(sampleRate);
    trackerEngine.reset();
}

void AudioEngine::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Audio thread — no allocation, no locking, no I/O.
    const bool isPlaying = transportState.getIsPlaying();
    if (!isPlaying && sequencerWasPlaying)
    {
        nesSynth.noteOff();
        trackerEngine.reset();
        sequencerWasPlaying = false;
    }

    int renderedSamples = 0;
    trackerEngine.processBlock(bufferToFill.numSamples,
                               transportState,
                               [this, &bufferToFill, &renderedSamples](const TrackerNoteEvent& event)
                               {
                                   const int eventOffset = std::clamp(event.sampleOffset,
                                                                      0,
                                                                      bufferToFill.numSamples);
                                   renderSynthSlice(bufferToFill,
                                                    renderedSamples,
                                                    eventOffset - renderedSamples);

                                   if (event.cell.isNoteOff())
                                       nesSynth.noteOff();
                                   else if (event.cell.hasNote())
                                       nesSynth.noteOn(event.cell.note);

                                   renderedSamples = eventOffset;
                               });

    sequencerWasPlaying = isPlaying;
    renderSynthSlice(bufferToFill, renderedSamples, bufferToFill.numSamples - renderedSamples);
}

void AudioEngine::releaseResources()
{
    // NES synth holds no external resources — nothing to release.
}

void AudioEngine::renderSynthSlice(const juce::AudioSourceChannelInfo& bufferToFill,
                                   int relativeStartSample,
                                   int numSamples)
{
    if (numSamples <= 0)
        return;

    juce::AudioSourceChannelInfo slice(bufferToFill.buffer,
                                       bufferToFill.startSample + relativeStartSample,
                                       numSamples);
    nesSynth.renderBlock(slice);
}
