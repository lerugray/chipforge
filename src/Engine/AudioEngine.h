#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "TransportState.h"
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
    void noteOn(int midiNote) { nesSynth.noteOn(midiNote); }
    void noteOff()            { nesSynth.noteOff(); }
    int  getActiveNote() const { return nesSynth.getActiveNote(); }

private:
    TransportState& transportState;
    NES2A03Synth    nesSynth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
