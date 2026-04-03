#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "TransportState.h"

//==============================================================================
// AudioEngine
//
// Owns the JUCE audio device and is the sole entry point for audio I/O.
// Phase 0: outputs a 440 Hz sine wave when the transport is playing.
//
// AUDIO THREAD RULES (enforced here and in all future processBlock calls):
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

private:
    TransportState& transportState;

    double currentSampleRate  = 44100.0;
    double currentAngle       = 0.0;
    double angleDelta         = 0.0;

    static constexpr double testToneFrequency = 440.0;
    static constexpr float  testToneAmplitude = 0.25f;  // -12 dBFS — not too loud

    void updateAngleDelta();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
