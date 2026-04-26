#include "AudioEngine.h"

#include <algorithm>

namespace
{
Pattern createStartupPattern()
{
    Pattern pattern(16, Pattern::DefaultTracks);
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
      trackerEngine(trackerPattern),
      pendingPatterns { createStartupPattern(), createStartupPattern() }
{
    for (auto& state : pendingPatternStates)
        state.store(PatternSlotFree, std::memory_order_relaxed);
    for (auto& sequence : pendingPatternSequences)
        sequence.store(0, std::memory_order_relaxed);

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
    consumePendingPatternLoad();

    const bool isPlaying = transportState.getIsPlaying();
    if (!isPlaying && sequencerWasPlaying)
    {
        nesSynth.noteOff();
        trackerEngine.reset();
        sequencerWasPlaying = false;
        playbackRow.store(-1, std::memory_order_relaxed);
    }

    consumePendingAuditionCommand();

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
    if (isPlaying)
        playbackRow.store(trackerEngine.getCurrentRow(), std::memory_order_relaxed);

    renderSynthSlice(bufferToFill, renderedSamples, bufferToFill.numSamples - renderedSamples);
}

void AudioEngine::releaseResources()
{
    // NES synth holds no external resources — nothing to release.
}

void AudioEngine::noteOn(int midiNote)
{
    pendingAuditionCommand.store(midiNote, std::memory_order_release);
}

void AudioEngine::noteOff()
{
    pendingAuditionCommand.store(AuditionNoteOff, std::memory_order_release);
}

bool AudioEngine::loadPatternForPlayback(const Pattern& pattern)
{
    for (int i = 0; i < PendingPatternSlotCount; ++i)
    {
        int expected = PatternSlotFree;
        if (!pendingPatternStates[static_cast<std::size_t>(i)].compare_exchange_strong(expected,
                                                                                       PatternSlotWriting,
                                                                                       std::memory_order_acquire,
                                                                                       std::memory_order_relaxed))
            continue;

        const bool copied = pendingPatterns[static_cast<std::size_t>(i)].copyCellsFrom(pattern);
        if (copied)
        {
            const int sequence = nextPatternSequence.fetch_add(1, std::memory_order_acq_rel) + 1;
            pendingPatternSequences[static_cast<std::size_t>(i)].store(sequence, std::memory_order_release);
        }

        pendingPatternStates[static_cast<std::size_t>(i)].store(copied ? PatternSlotReady : PatternSlotFree,
                                                                std::memory_order_release);
        return copied;
    }

    return false;
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

void AudioEngine::consumePendingAuditionCommand()
{
    const int command = pendingAuditionCommand.exchange(NoAuditionCommand, std::memory_order_acq_rel);
    if (command == AuditionNoteOff)
        nesSynth.noteOff();
    else if (command >= 0)
        nesSynth.noteOn(command);
}

void AudioEngine::consumePendingPatternLoad()
{
    int selectedSlot = -1;
    int selectedSequence = -1;

    for (int i = 0; i < PendingPatternSlotCount; ++i)
    {
        const auto slot = static_cast<std::size_t>(i);
        if (pendingPatternStates[slot].load(std::memory_order_acquire) != PatternSlotReady)
            continue;

        const int sequence = pendingPatternSequences[slot].load(std::memory_order_acquire);
        if (sequence > selectedSequence)
        {
            selectedSequence = sequence;
            selectedSlot = i;
        }
    }

    if (selectedSlot < 0)
        return;

    for (int i = 0; i < PendingPatternSlotCount; ++i)
    {
        if (i == selectedSlot)
            continue;

        const auto slot = static_cast<std::size_t>(i);
        if (pendingPatternStates[slot].load(std::memory_order_acquire) == PatternSlotReady)
            pendingPatternStates[slot].store(PatternSlotFree, std::memory_order_release);
    }

    const auto slot = static_cast<std::size_t>(selectedSlot);
    int expected = PatternSlotReady;
    if (!pendingPatternStates[slot].compare_exchange_strong(expected,
                                                            PatternSlotReading,
                                                            std::memory_order_acquire,
                                                            std::memory_order_relaxed))
        return;

    if (trackerPattern.copyCellsFrom(pendingPatterns[slot]))
    {
        trackerEngine.setPattern(trackerPattern);
        trackerEngine.reset();
        sequencerWasPlaying = false;
        playbackRow.store(-1, std::memory_order_relaxed);
    }

    pendingPatternStates[slot].store(PatternSlotFree, std::memory_order_release);
}
