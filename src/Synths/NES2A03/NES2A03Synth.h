#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "nes_apu/Nes_Apu.h"
#include "nes_apu/Blip_Buffer.h"
#include <atomic>
#include <vector>

//==============================================================================
// NES2A03Synth
//
// Wraps Nes_Snd_Emu's Nes_Apu to provide a simple note-playing interface for
// the NES 2A03 APU. Phase 1: keyboard drives Pulse 1.
//
// Thread safety:
//   noteOn/noteOff  — called from UI thread, communicate via atomic
//   renderBlock     — called from audio thread only
//   NO allocation, NO locking inside renderBlock
//==============================================================================
class NES2A03Synth
{
public:
    NES2A03Synth();
    ~NES2A03Synth() = default;

    // Call once before audio starts
    void prepare(double sampleRate, int maxBlockSize);

    // Audio thread — fill bufferToFill with NES audio
    void renderBlock(const juce::AudioSourceChannelInfo& bufferToFill);

    // UI thread — thread-safe note control (Pulse 1)
    void noteOn(int midiNote);
    void noteOff();

    int  getActiveNote() const { return activeNote.load(std::memory_order_relaxed); }

private:
    // NES APU and its output buffer
    Nes_Apu    apu;
    Blip_Buffer blipBuf;

    // Audio-thread state
    int  apuTime     { 0 };    // current clock time within frame (increments by 4 per register write)
    int  frameLength { 29780 };// NES CPU cycles per frame (alternates 29780/29781 for timing accuracy)
    bool prepared    { false };

    // Thread-safe pending command: -2=no-op, -1=noteOff, 0-127=noteOn
    std::atomic<int> pendingNote { -2 };
    std::atomic<int> activeNote  { -1 };

    // Pre-allocated temp buffer (sized in prepare(), never resized on audio thread)
    std::vector<blip_sample_t> tempBuffer;

    void applyNoteOn(int midiNote);
    void applyNoteOff();
    void writeReg(int addr, int data);  // writes at current apuTime, advances by 4

    // NES period lookup table (index 0 = A1 = MIDI 33, covers MIDI 33-112)
    // Source: Nes_Snd_Emu demo / NES APU hardware documentation
    static const uint8_t kPeriodLo[80];
    static const uint8_t kPeriodHi[80];
    static constexpr int kNoteTableOffset = 33; // table[0] corresponds to MIDI note 33 (A1)

    NES2A03Synth(const NES2A03Synth&) = delete;
    NES2A03Synth& operator=(const NES2A03Synth&) = delete;
};
