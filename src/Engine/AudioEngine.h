#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include <array>
#include <atomic>

#include "TransportState.h"
#include "TrackerEngine.h"
#include "Synths/NES2A03/NES2A03Synth.h"

//==============================================================================
// AudioEngine
//
// Owns the JUCE audio device and is the sole entry point for audio I/O.
// Phase 1: routes audio through the NES 2A03 synth.
//
// AUDIO THREAD RULES:
//   - No memory allocation (malloc/new/std::vector resize)
//   - No mutex locking
//   - No file I/O or DBG() calls
//   - All state shared with UI uses atomics or lock-free FIFOs
//==============================================================================
class AudioEngine : public juce::AudioAppComponent
{
public:
    explicit AudioEngine(TransportState& transport);
    ~AudioEngine() override;

    // juce::AudioAppComponent
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // UI thread — forward keyboard events to the NES synth
    void noteOn(int midiNote);
    void noteOff();
    int  getActiveNote() const { return nesSynth.getActiveNote(); }
    const Pattern& getPattern() const noexcept { return trackerPattern; }
    bool loadPatternForPlayback(const Pattern& pattern);
    int  getPlaybackRow() const noexcept { return playbackRow.load(std::memory_order_relaxed); }

private:
    static constexpr int PendingPatternSlotCount { 2 };
    static constexpr int PatternSlotFree         { 0 };
    static constexpr int PatternSlotWriting      { 1 };
    static constexpr int PatternSlotReady        { 2 };
    static constexpr int PatternSlotReading      { 3 };
    static constexpr int NoAuditionCommand       { -2 };
    static constexpr int AuditionNoteOff         { -1 };

    TransportState& transportState;
    Pattern          trackerPattern;
    TrackerEngine    trackerEngine;
    std::array<Pattern, PendingPatternSlotCount> pendingPatterns;
    std::array<std::atomic<int>, PendingPatternSlotCount> pendingPatternStates;
    std::array<std::atomic<int>, PendingPatternSlotCount> pendingPatternSequences;
    std::atomic<int> nextPatternSequence { 0 };
    std::atomic<int> playbackRow { -1 };
    std::atomic<int> pendingAuditionCommand { NoAuditionCommand };
    NES2A03Synth    nesSynth;
    bool sequencerWasPlaying { false };

    void consumePendingAuditionCommand();
    void consumePendingPatternLoad();
    void renderSynthSlice(const juce::AudioSourceChannelInfo& bufferToFill,
                          int relativeStartSample,
                          int numSamples);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
